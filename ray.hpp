// ================================================
// File: ray.hpp
// Created on: 2025-06-04 13:54:06
// Last modified: 2025-06-04 19:59:11
// Created by: Alwin R Ajeesh
// ================================================
#pragma once

#include <GLM/ext/vector_float3.hpp>
#include <GLM/geometric.hpp>
#include <GLM/glm.hpp>

class ray 
{

public:
	ray() { }
	ray(const glm::dvec3& vorigin, const glm::dvec3& vdirection) : origin(vorigin), direction(glm::normalize(vdirection)) { }
	
	const glm::dvec3& getOrigin() const { return origin; }
	const glm::dvec3& getDirection() const { return direction; }

	glm::dvec3 at(double t) const
	{
		return origin + t*direction;
	}

private:
	glm::dvec3 origin;
	glm::dvec3 direction;
};
