// ================================================
// File: sphere.hpp
// Created on: 2025-06-05 14:36:58
// Last modified: 2025-06-06 12:59:06
// Created by: Alwin R Ajeesh
// ================================================

#include "ray.hpp"
#include <GLM/ext/vector_double3.hpp>

class Sphere
{

public:
	Sphere() { }
	Sphere(const glm::dvec3& center, const double radius) : m_Center(center), m_Radius(radius) { }

	double intersect(const ray& r) const;
	glm::dvec3 getCenter() const { return m_Center; }

	
private:
	glm::dvec3 m_Center;
	double m_Radius;

};

double Sphere::intersect(const ray& r) const
{	
	glm::dvec3 oc = m_Center - r.getOrigin();
	double a = glm::dot((r.getDirection()), r.getDirection());
	double h = glm::dot(r.getDirection(), oc);
	double c = glm::dot(oc, oc) - m_Radius*m_Radius;
	double discriminant = h*h-a*c;
	if (discriminant < 0) {
		return -1;
	} else {
		//Closest t (hitpoint from camera)
		return (h - std::sqrt(discriminant)) / a;
	}
}
