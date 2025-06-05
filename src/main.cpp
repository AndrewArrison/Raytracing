// ================================================
// File: main.cpp
// Created on: 2025-06-03 21:17:33
// Last modified: 2025-06-05 15:13:41
// Created by: Alwin R Ajeesh
// ================================================
#include <GLM/common.hpp>
#include <GLM/ext/vector_double3.hpp>
#include <GLM/geometric.hpp>
#include <cmath>
#include <vector>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <iostream>
#include "ray.hpp"
#include "Timer.hpp"
#include "sphere.hpp"


int aspect_ratio = 16.0/9.0;
int image_width = 500;
int image_height = int(image_width / aspect_ratio);

int viewport_height = 2.0;
int viewport_width = viewport_height * (double(image_width)/image_height);

int window_width = 500;
int window_height = 500;


uint32_t pack_color(glm::dvec3& color)
{
	int ir = static_cast<int>(255.999 * color.x);
	int ig = static_cast<int>(255.999 * color.y);
	int ib = static_cast<int>(255.999 * color.z);

	return (ir << 16) | (ig << 8) | ib; // XRGB8888
}

glm::dvec3 ray_color(const ray& r, const std::vector<Sphere>& spheres)
{
	// double t = hit_sphere(glm::dvec3(0, 0, -1), 0.5, r);
	for (Sphere sphere : spheres) {
		double t = sphere.intersect(r);
		if (t > 0.0) {
			glm::dvec3 hit_point = r.at(t);
			glm::dvec3 Normal = glm::normalize(hit_point - glm::dvec3(0, 0, -1));
			glm::dvec3  light_pos = glm::dvec3(1, 1, 0.4);
			//vector from surface to light
			glm::dvec3 light_dir = glm::normalize(light_pos - hit_point);
			//normal dot light = cos(angle)
			double diffuse_intensity = glm::max(0.0, glm::dot(Normal, light_dir));
			//base colour of sphere
			glm::dvec3 albedo(0.8, 0.3, 0.1);
			//light color
			glm::dvec3 light_color(1.0, 0.5, 0.5);
			return 0.5*(Normal + glm::dvec3(1.0));
			// return albedo * light_color * diffuse_intensity;
		}
	}
	glm::dvec3 unit_direction = r.getDirection();
	double blend_factor = 0.5*(unit_direction.y + 1.0);
	glm::dvec3 color = (1.0-blend_factor)*glm::dvec3(1.0, 1.0, 1.0) + blend_factor*glm::dvec3(0.5, 0.7, 1.0);
	return color;
}

void render(SDL_Renderer* renderer, SDL_Texture* texture, glm::dvec3& camera_center, glm::dvec3& pixel00_loc, glm::dvec3& pixel_delta_w, glm::dvec3& pixel_delta_h, SDL_FRect& rect, std::vector<Sphere> spheres)
{
	//pixel buffer
	uint32_t* pixels = new uint32_t[image_width * image_height];
	//Fill pixel buffer with image data
	Timer time;
	for (int j = 0; j < image_height; j++) {
		for (int i = 0; i < image_width; i++) {
			glm::dvec3 pixel_center = pixel00_loc + ( double(i) * pixel_delta_w) + ( double(j) * pixel_delta_h);
			glm::dvec3 ray_direction = pixel_center - camera_center;
			ray r(camera_center, ray_direction);
			glm::dvec3 pixel_color = ray_color(r, spheres);
			pixels[j * image_width + i] = pack_color(pixel_color);
		}
		// std::clog << "\rScanlines remaining : " << (image_height - j) << " " << std::flush;
	}
	SDL_UpdateTexture(texture, nullptr, pixels, image_width * sizeof(uint32_t));
	SDL_RenderTexture(renderer, texture, nullptr, &rect);
	SDL_RenderPresent(renderer);
	delete[] pixels;
	// std::clog << "\rDone.                        \n";
}

	
int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
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
	
	int focal_length = 1.0;
	glm::dvec3 camera_center = glm::dvec3(0);
	glm::dvec3 viewport_w = glm::dvec3(viewport_width, 0, 0);
	glm::dvec3 viewport_h = glm::dvec3(0, -viewport_height, 0);
	glm::dvec3 pixel_delta_w = viewport_w / double(image_width);
	glm::dvec3 pixel_delta_h = viewport_h / double(image_height);
	glm::dvec3 viewport_ul = camera_center - glm::dvec3(0, 0, focal_length) - viewport_w/2.0 - viewport_h/2.0;
	glm::dvec3 pixel00_loc = viewport_ul + 0.5*(pixel_delta_w + pixel_delta_h);
	SDL_FRect rect = {0, 0, 400, 400};

	std::vector<Sphere> Spheres;
	Spheres.push_back(Sphere(glm::dvec3(0.0, 0.0, -1.0), 0.5));
	Spheres.push_back(Sphere(glm::dvec3(0.7, 0.5, -1.5), 0.5));

	render(renderer, texture, camera_center, pixel00_loc, pixel_delta_w, pixel_delta_h, rect, Spheres);

	bool running = true;
	while(running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;
				case SDL_EVENT_KEY_DOWN:
					switch (event.key.scancode) {
						case SDL_SCANCODE_R:
							render(renderer, texture, camera_center, pixel00_loc, pixel_delta_w, pixel_delta_h, rect, Spheres);
							break;
						default:
							break;
					}
				default:
					break;
			}
		}
		// SDL_RenderClear(renderer);
		// SDL_RenderTexture(renderer, texture, nullptr, nullptr);
		// SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	return 0;
}

