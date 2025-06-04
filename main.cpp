// ================================================
// File: main.cpp
// Created on: 2025-06-03 21:17:33
// Last modified: 2025-06-04 12:33:58
// Created by: Alwin R Ajeesh
// ================================================
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <iostream>

int image_width = 2560;
int image_height = 2560;

int window_width = 640;
int window_height = 480;

uint32_t pack_color(double r, double g, double b)
{
	int ir = static_cast<int>(255.999 * r);
	int ig = static_cast<int>(255.999 * g);
	int ib = static_cast<int>(255.999 * b);

	return (ir << 16) | (ig << 8) | ib; // XRGB8888
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
	
	//pixel buffer
	uint32_t* pixels = new uint32_t[image_width * image_height];
	//Fill pixel buffer with image data
	for (int j = 0; j < image_height; j++) {
		std::clog << "\rScanlines remaining : " << (image_height - j) << " " << std::flush;
		for (int i = 0; i < image_width; i++) {
			double r = double(i) / image_width;
			double g = double(j) / image_height;
			double b = 0.2;
			
			pixels[j * image_width + i] = pack_color(r, g, b);
		}
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

