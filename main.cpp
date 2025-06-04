// ================================================
// File: main.cpp
// Created on: 2025-06-03 21:17:33
// Last modified: 2025-06-04 21:39:09
// Created by: Alwin R Ajeesh
// ================================================
#include <GLM/common.hpp>
#include <GLM/ext/vector_double3.hpp>
#include <GLM/geometric.hpp>
#include <cmath>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <iostream>
#include "ray.hpp"


int aspect_ratio = 16.0/9.0;
int image_width = 400;
int image_height = int(image_width / aspect_ratio);

int viewport_height = 2.0;
int viewport_width = viewport_height * (double(image_width)/image_height);

int window_width = 500;
int window_height = 500;

double hit_sphere(const glm::dvec3& center, double radius, const ray& r)
{
	glm::dvec3 oc = center - r.getOrigin();
	double a = glm::dot((r.getDirection()), r.getDirection());
	double h = glm::dot(r.getDirection(), oc);
	double c = glm::dot(oc, oc) - radius*radius;
	double discriminant = h*h-a*c;
	if (discriminant < 0) {
		return -1;
	} else {
		return (h - std::sqrt(discriminant)) / a;
	}
}

uint32_t pack_color(glm::dvec3& color)
{
	int ir = static_cast<int>(255.999 * color.x);
	int ig = static_cast<int>(255.999 * color.y);
	int ib = static_cast<int>(255.999 * color.z);

	return (ir << 16) | (ig << 8) | ib; // XRGB8888
}

glm::dvec3 ray_color(const ray& r)
{
	double t = hit_sphere(glm::dvec3(0, 0, -1), 0.5, r);
	if (t > 0.0) {
		glm::dvec3 Normal = glm::normalize(r.at(t) - glm::dvec3(0, 0, -1));
		return 0.5*(Normal + glm::dvec3(1.0));
	}
	glm::dvec3 unit_direction = r.getDirection();
	double blend_factor = 0.5*(unit_direction.y + 1.0);
	glm::dvec3 color = (1.0-blend_factor)*glm::dvec3(1.0, 1.0, 1.0) + blend_factor*glm::dvec3(0.5, 0.7, 1.0);
	return color;
}

void render(SDL_Renderer* renderer, SDL_Texture* texture, glm::dvec3& camera_center, glm::dvec3& pixel00_loc, glm::dvec3& pixel_delta_w, glm::dvec3& pixel_delta_h, SDL_FRect& rect)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	//pixel buffer
	uint32_t* pixels = new uint32_t[image_width * image_height];
	//Fill pixel buffer with image data
	for (int j = 0; j < image_height; j++) {
		for (int i = 0; i < image_width; i++) {
			glm::dvec3 pixel_center = pixel00_loc + ( double(i) * pixel_delta_w) + ( double(j) * pixel_delta_h);
			glm::dvec3 ray_direction = pixel_center - camera_center;
			ray r(camera_center, ray_direction);
			glm::dvec3 pixel_color = ray_color(r);
			pixels[j * image_width + i] = pack_color(pixel_color);
		}
		SDL_UpdateTexture(texture, nullptr, pixels, image_width * sizeof(uint32_t));
		SDL_RenderTexture(renderer, texture, nullptr, &rect);
		SDL_RenderPresent(renderer);
		std::clog << "\rScanlines remaining : " << (image_height - j) << " " << std::flush;
	}
	delete[] pixels;
	std::clog << "\rDone.                        \n";
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
	camera_center.z += 1;
	glm::dvec3 viewport_w = glm::dvec3(viewport_width, 0, 0);
	glm::dvec3 viewport_h = glm::dvec3(0, -viewport_height, 0);
	glm::dvec3 pixel_delta_w = viewport_w / double(image_width);
	glm::dvec3 pixel_delta_h = viewport_h / double(image_height);
	glm::dvec3 viewport_ul = camera_center - glm::dvec3(0, 0, focal_length) - viewport_w/2.0 - viewport_h/2.0;
	glm::dvec3 pixel00_loc = viewport_ul + 0.5*(pixel_delta_w + pixel_delta_h);
	SDL_FRect rect = {0, 0, 400, 400};

	render(renderer, texture, camera_center, pixel00_loc, pixel_delta_w, pixel_delta_h, rect);

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
							render(renderer, texture, camera_center, pixel00_loc, pixel_delta_w, pixel_delta_h, rect);
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

