// ================================================
// File: hittable.hpp
// Created on: 2025-06-06 12:22:52
// Last modified: 2025-06-06 12:32:19
// Created by: Alwin R Ajeesh
// ================================================

#include "ray.hpp"

struct HitRecord {
	glm::dvec3 point;
	glm::dvec3 normal;
	double t;
};

class Hitable
{
public:
	virtual bool intersect(const ray& r, double t_min, double t_max, HitRecord& rec) const = 0;
	virtual ~Hitable() = default;
};
