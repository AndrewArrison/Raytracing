#include <GLM/ext/vector_double3.hpp>
#include <GLM/geometric.hpp>
#include <GLM/trigonometric.hpp>
#include <cmath>
#include <vector>
#include <thread>
#include <functional>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <iostream>
#include "ray.hpp"
#include "Timer.hpp"
#include "sphere.hpp"

float aspect_ratio = 16.0f / 9.0f;
float image_width = 600.0f;
float image_height = int(image_width / aspect_ratio);

double fov_deg = 45.0;
double fov_rad = glm::radians(fov_deg);
double focal_length = 1.0;
double viewport_height = 2.0 * focal_length * tan(fov_rad / 2.0);
double viewport_width = viewport_height * (double(image_width) / image_height);

int window_width = image_width;
int window_height = image_height;

uint32_t pack_color(glm::dvec3& color)
{
	int ir = static_cast<int>(255.999 * color.x);
	int ig = static_cast<int>(255.999 * color.y);
	int ib = static_cast<int>(255.999 * color.z);
	return (ir << 16) | (ig << 8) | ib;
}

glm::dvec3 ray_color(const ray& r, const std::vector<Sphere>& spheres)
{
	for (Sphere sphere : spheres) {
		double t = sphere.intersect(r);
		if (t > 0.0) {
			glm::dvec3 hit_point = r.at(t);
			glm::dvec3 Normal = glm::normalize(hit_point - sphere.getCenter());
			glm::dvec3 light_pos = glm::dvec3(1, 1, 0.4);
			glm::dvec3 light_dir = glm::normalize(light_pos - hit_point);
			double diffuse_intensity = glm::max(0.0, glm::dot(Normal, light_dir));
			glm::dvec3 albedo(0.8, 0.3, 0.1);
			glm::dvec3 light_color(1.0, 1.0, 1.0);
			return albedo * light_color * diffuse_intensity;
		}
	}
	// glm::dvec3 unit_direction = r.getDirection();
	// double blend_factor = 0.5 * (unit_direction.y + 1.0);
	// return (1.0 - blend_factor) * glm::dvec3(1.0, 1.0, 1.0) + blend_factor * glm::dvec3(0.5, 0.7, 1.0);
	return glm::dvec3(0, 0, 0);
}

void render_rows(int start_y, int end_y, uint32_t* pixels, glm::dvec3& camera_center,
	glm::dvec3& pixel00_loc, glm::dvec3& pixel_delta_w, glm::dvec3& pixel_delta_h,
	const std::vector<Sphere>& spheres)
{
	for (int j = start_y; j < end_y; ++j) {
		for (int i = 0; i < image_width; ++i) {
			glm::dvec3 pixel_center = pixel00_loc + (double(i) * pixel_delta_w) + (double(j) * pixel_delta_h);
			glm::dvec3 ray_direction = pixel_center - camera_center;
			ray r(camera_center, ray_direction);
			glm::dvec3 pixel_color = ray_color(r, spheres);
			pixels[j * int(image_width) + i] = pack_color(pixel_color);
		}
	}
}

void render(SDL_Renderer* renderer, SDL_Texture* texture, glm::dvec3& camera_center,
	glm::dvec3& pixel00_loc, glm::dvec3& pixel_delta_w, glm::dvec3& pixel_delta_h,
	SDL_FRect& rect, std::vector<Sphere> spheres)
{
	uint32_t* pixels = new uint32_t[int(image_width * image_height)];

	// Multithreaded rendering
	// unsigned int thread_count = std::thread::hardware_concurrency();
	unsigned int thread_count = 4;
	// if (thread_count == 0) thread_count = 4; // fallback
	int rows_per_thread = int(image_height) / thread_count;

	std::vector<std::thread> threads;
	for (unsigned int t = 0; t < thread_count; ++t) {
		int start_y = t * rows_per_thread;
		int end_y = (t == thread_count - 1) ? int(image_height) : (t + 1) * rows_per_thread;
		threads.emplace_back(render_rows, start_y, end_y, pixels,
			std::ref(camera_center), std::ref(pixel00_loc),
			std::ref(pixel_delta_w), std::ref(pixel_delta_h),
			std::ref(spheres));
	}
	for (auto& t : threads) {
		t.join();
	}

	SDL_UpdateTexture(texture, nullptr, pixels, image_width * sizeof(uint32_t));
	SDL_RenderTexture(renderer, texture, nullptr, &rect);
	SDL_RenderPresent(renderer);
	delete[] pixels;
}

void camera_update_front(double yaw, double pitch, glm::dvec3& front)
{
	double rad_yaw = glm::radians(yaw);
	double rad_pitch = glm::radians(pitch);
	front.x = cos(rad_yaw) * cos(rad_pitch);
	front.y = sin(rad_pitch);
	front.z = sin(rad_yaw) * cos(rad_pitch);
	front = glm::normalize(front);
}

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL init failed\n" << SDL_GetError() << "\n";
		return 1;
	}
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;

	if (!SDL_CreateWindowAndRenderer("ray", window_width, window_height, 0, &window, &renderer)) {
		std::cerr << "window or renderer fail.\n" << SDL_GetError() << "\n";
		SDL_Quit();
		return 1;
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, image_width, image_height);

	double yaw = -90.0;
	double pitch = 0.0;
	glm::dvec3 camera_center = glm::dvec3(0);
	glm::dvec3 camera_front = glm::dvec3(0.0, 0.0, -1.0);
	glm::dvec3 camera_up = glm::dvec3(0.0, 1.0, 0.0);
	glm::dvec3 camera_right = glm::normalize(glm::cross(camera_front, camera_up));
	glm::dvec3 viewport_w = glm::dvec3(viewport_width, 0, 0);
	glm::dvec3 viewport_h = glm::dvec3(0, -viewport_height, 0);
	glm::dvec3 pixel_delta_w = viewport_w / double(image_width);
	glm::dvec3 pixel_delta_h = viewport_h / double(image_height);
	glm::dvec3 viewport_ul = camera_center - glm::dvec3(0, 0, focal_length) - viewport_w / 2.0 - viewport_h / 2.0;
	glm::dvec3 pixel00_loc = viewport_ul + 0.5 * (pixel_delta_w + pixel_delta_h);
	SDL_FRect rect = { 0.0f, 0.0f, image_width, image_height };

	camera_update_front(yaw, pitch, camera_front);

	std::vector<Sphere> Spheres = {
		Sphere(glm::dvec3(0.0, 0.0, -1.0), 0.5),
	};

	render(renderer, texture, camera_center, pixel00_loc, pixel_delta_w, pixel_delta_h, rect, Spheres);

	bool running = true;
	while (running) {
		camera_right = glm::normalize(glm::cross(camera_front, camera_up));
		viewport_ul = camera_center - glm::dvec3(0, 0, focal_length) - viewport_w / 2.0 - viewport_h / 2.0;
		pixel00_loc = viewport_ul + 0.5 * (pixel_delta_w + pixel_delta_h);

		SDL_Event event;
		render(renderer, texture, camera_center, pixel00_loc, pixel_delta_w, pixel_delta_h, rect, Spheres);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				running = false;
				break;
			case SDL_EVENT_MOUSE_MOTION:
				yaw += event.motion.xrel * 0.1;
				camera_update_front(yaw, pitch, camera_front);
				break;
			case SDL_EVENT_KEY_DOWN:
				switch (event.key.scancode) {
				case SDL_SCANCODE_ESCAPE:
					running = false;
					break;
				case SDL_SCANCODE_R:
					render(renderer, texture, camera_center, pixel00_loc, pixel_delta_w, pixel_delta_h, rect, Spheres);
					break;
				case SDL_SCANCODE_W:
					camera_center += camera_front * 0.1;
					break;
				case SDL_SCANCODE_S:
					camera_center -= camera_front * 0.1;
					break;
				case SDL_SCANCODE_A:
					camera_center -= camera_right * 0.1;
					break;
				case SDL_SCANCODE_D:
					camera_center += camera_right * 0.1;
					break;
				case SDL_SCANCODE_Q:
					camera_center += camera_up * 0.1;
					break;
				case SDL_SCANCODE_E:
					camera_center -= camera_up * 0.1;
					break;
				default:
					break;
				}
			default:
				break;
			}
		}
		SDL_Delay(16);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	return 0;
}
