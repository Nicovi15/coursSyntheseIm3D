#pragma once
#include "Ray.h"
#include <optional>


class Sphere
{
public :
	Vec3 centre;
	float radius;
	Vec3 color;

	//essayer de remplacer par optional<float>
	float intersect(Ray r)
	{                
		// returns distance, 0 if nohit
		// Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
		Vec3 op = centre - r.origin;        
		
		float t, eps = 1e-4, 
			b = op.dot(r.direction), 
			det = b * b - op.dot(op) + radius * radius;

		if (det < 0)
			return 0;
		else
			det = sqrt(det);
		return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
	}

};

