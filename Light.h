#pragma once
#ifndef _LIGHT_
#define _LIGHT_
#include <iostream>
#include "Vec3.h"

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

		return intensity / W0.normSquared() * f;
	}

	Vec3 getLightOut(Vec3 point, Vec3 posO, Vec3 posSphere, Vec3 albedo)
	{
		Vec3 W0 = position - point;
		Vec3 N = (point - posSphere).norm();
		/// f = fonction de la surface
		float f = N.dot(W0.norm()) / 3.14f;

		return intensity / W0.normSquared() * f * (albedo * color);
	}
	
	/*
	int estOcculte( Vec3 point, vector<Sphere> objetsScene) {

		Vec3 pointLum = position - point;
		Vec3 dirVersLum = pointLum.unitVector();
		//float distancePointLum = sqrt(pointLum.normSquared());
		float distancePointLum = pointLum.normSquared();
		Ray r{ point, dirVersLum };

		for (int i = 0; i < objetsScene.size(); i++)
		{
			float inter = objetsScene[i].intersect(r);
			if (inter > 0 && inter * inter < distancePointLum)
				return 0;
		}

		return 1;
	}
	*/
};


#endif;