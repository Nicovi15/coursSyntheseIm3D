#pragma once
#include "Vec3.h"
#include <iostream>

class Light
{

public :
	Vec3 position;
	float intensity;
	Vec3 color;


	float getLightOut(Vec3 point, Vec3 posO, Vec3 posSphere) 
	{
		Vec3 W0 = position - point;
		Vec3 N = (point - posSphere).norm();
		float f = N.dot(W0.norm()) / 3.14f;

		//float f = ((N.dot(W0)) / 3.14f);

		//return (intensity * f) / W0.normSquared();

		//std::cout << f << std::endl;
		return intensity / W0.normSquared() * f; // *albedo

		//return intensity / W0.normSquared() * cos;
	}

	Vec3 getLightOut(Vec3 point, Vec3 posO, Vec3 posSphere, Vec3 albedo)
	{
		Vec3 W0 = position - point;
		Vec3 N = (point - posSphere).norm();
		float f = N.dot(W0.norm()) / 3.14f;

		//float f = ((N.dot(W0)) / 3.14f);

		//return (intensity * f) / W0.normSquared();

		//std::cout << f << std::endl;
		return intensity / W0.normSquared() * f * (albedo * color);

		//return intensity / W0.normSquared() * cos;
	}

};

