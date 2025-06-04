// ================================================
// File: main.cpp
// Created on: 2025-06-03 21:17:33
// Last modified: 2025-06-04 15:22:27
// Created by: Alwin R Ajeesh
// ================================================
#include "ray.hpp"
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <iostream>

int window_width = 640;
int window_height = 480;

int aspect_ratio = 16.0/9.0;
int image_width = 256;

int image_height = int(image_width / aspect_ratio);
int viewport_height = 2.0;
int viewport_width = viewport_height * (double(image_width)/image_height);

uint32_t pack_color(glm::dvec3& color)
{
	int ir = static_cast<int>(255.999 * color.x);
	int ig = static_cast<int>(255.999 * color.y);
	int ib = static_cast<int>(255.999 * color.z);

	return (ir << 16) | (ig << 8) | ib; // XRGB8888
}

glm::dvec3 ray_color(const ray& r)
{
	glm::dvec3 unit_direction = glm::normalize(r.getDirection());
	double blend_factor = 0.5*(unit_direction.y + 1.0);
	glm::dvec3 color = (1.0-blend_factor)*glm::dvec3(1.0, 1.0, 1.0) + blend_factor*glm::dvec3(0.5, 0.7, 1.0);
	return color;
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
		std::clog << "\rScanlines remaining : " << (image_height - j) << " " << std::flush;
	}
	
	std::clog << "\r Done.                        \n";

	SDL_UpdateTexture(texture, nullptr, pixels, image_width * sizeof(uint32_t));

	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
	
	bool running = true;
	while(running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;

				default:
					break;
			}
		}
		// SDL_RenderClear(renderer);
		// SDL_RenderTexture(renderer, texture, nullptr, nullptr);
		// SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}

	delete[] pixels;
	SDL_DestroyTexture(texture);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	return 0;
}

