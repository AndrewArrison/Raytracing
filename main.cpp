// ================================================
// File: main.cpp
// Created on: 2025-06-03 21:17:33
// Last modified: 2025-06-03 22:18:56
// Created by: Alwin R Ajeesh
// ================================================

#include <iostream>

int image_width = 2560;
int image_height = 2560;

void render()
{
	std::cout << "P3\n" << image_width << " " << image_height << "\n" << "255\n";

	for (int j = 0; j < image_height; j++) {
		for (int i = 0; i < image_width; i++) {
			double r = (double)(i) / (double)(image_width);
			double g = (double)(j) / (double)(image_height);
			double b = 0.2;

			int ir = int(255.999*r);
			int ig = int(255.999*g);
			int ib = int(255.999*b);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}	
}

int main()
{
	render();
	return 0;
}

