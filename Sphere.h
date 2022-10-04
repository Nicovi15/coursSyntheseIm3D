#pragma once
#ifndef _SPHERE_
#define _SPHERE_
#include <string>
#include "Ray.h"
#include <optional>



class Sphere
{
public :
	Vec3 centre;
	float radius;
	Vec3 color;
	std::string tag;

	//essayer de remplacer par optional<float>
	//float intersect(Ray r)
	//{                
	//	// returns distance, 0 if nohit
	//	// Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
	//	Vec3 op = centre - r.origin;        
	//	
	//	float t, eps = 1e-4, 
	//		b = op.dot(r.direction), 
	//		det = b * b - op.dot(op) + radius * radius;
	//
	//	if (det < 0)
	//		return 0; //return std::nullopt;
	//	else
	//		det = sqrt(det);
	//	return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
	//}

	std::optional<float> intersect(Ray r)
	{
		// returns distance, 0 if nohit
		// Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
		Vec3 op = centre - r.origin;
	
		float t, eps = 1e-4,
			b = op.dot(r.direction),
			det = b * b - op.dot(op) + radius * radius;
	
		if (det < 0)
			return std::nullopt;
		else
			det = sqrt(det);

		float res = (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);

		if(res == 0)
			return std::nullopt;
		else
			return std::optional{ res };
	}

};

#endif;