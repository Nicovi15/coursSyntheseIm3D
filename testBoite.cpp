#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "Vec3.h"
#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"
#include "Light.h"
#include <vector>
#include <map>
#include <chrono>

using namespace cv;
using namespace std;

using namespace std::chrono_literals;

template <class DT = std::chrono::milliseconds,
	class ClockT = std::chrono::steady_clock>
	class Timer
{
	using timep_t = decltype(ClockT::now());

	timep_t _start = ClockT::now();
	timep_t _end = {};

public:
	void tick() {
		_end = timep_t{};
		_start = ClockT::now();
	}

	void tock() {
		_end = ClockT::now();
	}

	template <class duration_t = DT>
	auto duration() const {
		// Use gsl_Expects if your project supports it.
		assert(_end != timep_t{} && "Timer must toc before reading the time");
		return std::chrono::duration_cast<duration_t>(_end - _start);
	}
};


int clampColor(float x, float coef) {
	if ((x / coef) > coef)
		return 255;
	else if (x < 0)
		return 0;
	else
		return (int)(255 * (x / coef));
}

int estOcculte(Light l, Vec3 point, vector<Sphere> objetsScene, float eps) {
	Vec3 pointLum = l.position - point;
	Vec3 dirVersLum = pointLum.unitVector();
	float distancePointLum = pointLum.normSquared();
	Ray r{ point + dirVersLum * eps, dirVersLum };

	for (int i = 0; i < objetsScene.size(); i++)
	{
		optional<float> inter = objetsScene[i].intersect(r);

		if (inter.has_value() && inter.value() * inter.value() < distancePointLum)
			return 0;
	}

	return 1;
}

struct Boite {

	Vec3 min;
	Vec3 max;
	vector<Sphere> spheres;
	//string name;

	bool intersect(const Ray& r)
	{
		float tmin = (min.x - r.origin.x) / r.direction.x;
		float tmax = (max.x - r.origin.x) / r.direction.x;

		if (tmin > tmax) swap(tmin, tmax);

		float tymin = (min.y - r.origin.y) / r.direction.y;
		float tymax = (max.y - r.origin.y) / r.direction.y;

		if (tymin > tymax) swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax))
			return false;

		if (tymin > tmin)
			tmin = tymin;

		if (tymax < tmax)
			tmax = tymax;

		float tzmin = (min.z - r.origin.z) / r.direction.z;
		float tzmax = (max.z - r.origin.z) / r.direction.z;

		if (tzmin > tzmax) swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;

		if (tzmin > tmin)
			tmin = tzmin;

		if (tzmax < tmax)
			tmax = tzmax;

		return true;
	}

	bool operator==(const Boite& other) const
	{
		return (min.x == other.min.x && min.y == other.min.y && min.z == other.min.z &&
			 max.x == other.max.x && max.y == other.max.y && max.z == other.max.z);
	}
};

struct ArbreBinBoite {

	Boite current;
	ArbreBinBoite *left;
	ArbreBinBoite *right;

	void creerEnfants() {
		float dimX = current.max.x - current.min.x;
		float dimY = current.max.y - current.min.y;
		float dimZ = current.max.z - current.min.z;

		if (dimX > dimY && dimX > dimZ) 
		{
			// on coupe sur X
			float limite = current.min.x + dimX / 2.0f;
			left = new ArbreBinBoite{ Boite{ {current.min.x, current.min.y, current.min.z}, {limite, current.max.y, current.max.z} } };
			right = new ArbreBinBoite{ Boite{ {limite, current.min.y, current.min.z}, {current.max.x, current.max.y, current.max.z} } };

			for (int i = 0; i < current.spheres.size(); i++) {
				if (current.spheres[i].centre.x < limite)
					left->current.spheres.push_back(current.spheres[i]);
				else
					right->current.spheres.push_back(current.spheres[i]);
			}
		}
		else if (dimY > dimX && dimY > dimZ)
		{
			// on coupe sur Y
			float limite = current.min.y + dimY / 2.0f;
			left = new ArbreBinBoite{ Boite{ {current.min.x, current.min.y, current.min.z}, {current.max.x, limite, current.max.z} } };
			right =new ArbreBinBoite{ Boite{ {current.min.x, limite, current.min.z}, {current.max.x, current.max.y, current.max.z} } };

			for (int i = 0; i < current.spheres.size(); i++) {
				if (current.spheres[i].centre.y < limite)
					left->current.spheres.push_back(current.spheres[i]);
				else
					right->current.spheres.push_back(current.spheres[i]);
			}
		}
		else
		{
			// on coupe sur Z
			float limite = current.min.z + dimZ / 2.0f;
			left = new ArbreBinBoite{ Boite{ {current.min.x, current.min.y, current.min.z}, {current.max.x, current.max.y, limite} } };
			right = new ArbreBinBoite{ Boite{ {current.min.x, current.min.y, limite}, {current.max.x, current.max.y, current.max.z} } };

			for (int i = 0; i < current.spheres.size(); i++) {
				if (current.spheres[i].centre.z < limite)
					left->current.spheres.push_back(current.spheres[i]);
				else
					right->current.spheres.push_back(current.spheres[i]);
			}
		}

	}

};

void creerEnfants(ArbreBinBoite* pere, int n) 
{
	cout << "pere : " << pere->current.spheres.size() << endl;

	if(n == 0)
		return;

	float dimX = pere->current.max.x - pere->current.min.x;
	float dimY = pere->current.max.y - pere->current.min.y;
	float dimZ = pere->current.max.z - pere->current.min.z;

	//cout << dimX << endl;
	//cout << dimY << endl;
	//cout << dimZ << endl;

	ArbreBinBoite* childLeft = new ArbreBinBoite{};
	ArbreBinBoite* childRight = new ArbreBinBoite{};

	vector<Sphere> sleft;
	vector<Sphere> sright;

	if (dimX > dimY && dimX > dimZ) 
	{
		// on coupe sur X
		float limite = pere->current.min.x + dimX / 2.0f;
		//childLeft->current = Boite{{pere->current.min.x, pere->current.min.y, pere->current.min.z}, {limite, pere->current.max.y, pere->current.max.z}};
		//childRight->current = Boite{{limite, pere->current.min.y, pere->current.min.z}, {pere->current.max.x, pere->current.max.y, pere->current.max.z}};

		for (int i = 0; i < pere->current.spheres.size(); i++) {
			if (pere->current.spheres[i].centre.x < limite)
				sleft.push_back(pere->current.spheres[i]);
			else
				sright.push_back(pere->current.spheres[i]);
		}
	}
	else if (dimY > dimX && dimY > dimZ)
	{
		// on coupe sur Y
		float limite = pere->current.min.y + dimY / 2.0f;
		//childLeft->current = Boite{ {pere->current.min.x, pere->current.min.y, pere->current.min.z}, {pere->current.max.x, limite, pere->current.max.z} };
		//childRight->current = Boite{ {pere->current.min.x, limite, pere->current.min.z}, {pere->current.max.x, pere->current.max.y, pere->current.max.z}};

		for (int i = 0; i < pere->current.spheres.size(); i++) {
			if (pere->current.spheres[i].centre.y < limite)
				sleft.push_back(pere->current.spheres[i]);
			else
				sright.push_back(pere->current.spheres[i]);
		}
	}
	else
	{
		// on coupe sur Z
		float limite = pere->current.min.z + dimZ / 2.0f;
		//childLeft = new ArbreBinBoite{ Boite{ {pere->current.min.x, pere->current.min.y, pere->current.min.z}, {pere->current.max.x, pere->current.max.y, limite} } };
		//childRight = new ArbreBinBoite{ Boite{ {pere->current.min.x, pere->current.min.y, limite}, {pere->current.max.x, pere->current.max.y, pere->current.max.z} } };

		for (int i = 0; i < pere->current.spheres.size(); i++) {
			if (pere->current.spheres[i].centre.z < limite)
				sleft.push_back(pere->current.spheres[i]);
			else
				sright.push_back(pere->current.spheres[i]);
		}
	}

	if (sleft.size() > 0) 
	{
		childLeft->current.spheres = sleft;
		Boite box;
		float minX = 999999, minY = 999999, minZ = 999999;
		float maxX = -999999, maxY = -999999, maxZ = -999999;
		for (int i = 0; i < sleft.size(); i++)
		{
			if (sleft[i].centre.x - sleft[i].radius < minX)
				minX = sleft[i].centre.x - sleft[i].radius;
			if (sleft[i].centre.x + sleft[i].radius > maxX)
				maxX = sleft[i].centre.x + sleft[i].radius;

			if (sleft[i].centre.y - sleft[i].radius < minY)
				minY = sleft[i].centre.y - sleft[i].radius;
			if (sleft[i].centre.y + sleft[i].radius > maxY)
				maxY = sleft[i].centre.y + sleft[i].radius;

			if (sleft[i].centre.z - sleft[i].radius < minZ)
				minZ = sleft[i].centre.z - sleft[i].radius;
			if (sleft[i].centre.z + sleft[i].radius > maxZ)
				maxZ = sleft[i].centre.z + sleft[i].radius;
		}
		childLeft->current.min = { minX, minY, minZ };
		childLeft->current.max = { maxX, maxY, maxZ };
		pere->left = childLeft;
		creerEnfants(childLeft, n - 1);
	}
	else
		pere->left = NULL;

	if (sright.size() > 0)
	{
		childRight->current.spheres = sright;
		Boite box;
		float minX = 999999, minY = 999999, minZ = 999999;
		float maxX = -999999, maxY = -999999, maxZ = -999999;
		for (int i = 0; i < sright.size(); i++)
		{
			if (sright[i].centre.x - sright[i].radius < minX)
				minX = sright[i].centre.x - sright[i].radius;
			if (sright[i].centre.x + sright[i].radius > maxX)
				maxX = sright[i].centre.x + sright[i].radius;

			if (sright[i].centre.y - sright[i].radius < minY)
				minY = sright[i].centre.y - sright[i].radius;
			if (sright[i].centre.y + sright[i].radius > maxY)
				maxY = sright[i].centre.y + sright[i].radius;

			if (sright[i].centre.z - sright[i].radius < minZ)
				minZ = sright[i].centre.z - sright[i].radius;
			if (sright[i].centre.z + sright[i].radius > maxZ)
				maxZ = sright[i].centre.z + sright[i].radius;
		}
		childRight->current.min = { minX, minY, minZ };
		childRight->current.max = { maxX, maxY, maxZ };
		pere->right = childRight;
		creerEnfants(childRight, n - 1);
	}
	else
		pere->right = NULL;
	
}

vector<Sphere> getSphereInBox(ArbreBinBoite* arbre, Ray r) 
{
	vector<Sphere> res;

	if (!arbre->current.intersect(r)) {
		return res;
	}	

	vector<Sphere> spheresGauche;
	vector<Sphere> spheresDroit;

	if (!arbre->left && !arbre->right) {
		for (int i = 0; i < arbre->current.spheres.size(); i++)
			res.push_back(arbre->current.spheres[i]);

		return res;
	}

	if (arbre->left != NULL) {
		spheresGauche = getSphereInBox(arbre->left, r);
		for (int i = 0; i < spheresGauche.size(); i++)
			res.push_back(spheresGauche[i]);
	}
	
	if (arbre->right != NULL) {
		spheresDroit = getSphereInBox(arbre->right, r);
		for (int i = 0; i < spheresDroit.size(); i++)
			res.push_back(spheresDroit[i]);
	}

	return res;
	
	//if (spheresGauche.size() == 0) 
	//	return spheresDroit;
	//
	//if (spheresDroit.size() == 0)
	//	return spheresGauche;
	//
	//cout << spheresDroit.size() << endl;
	//res.reserve(spheresGauche.size() + spheresDroit.size()); // preallocate memory
	//res.insert(res.end(), spheresGauche.begin(), spheresGauche.end());
	//res.insert(res.end(), spheresDroit.begin(), spheresDroit.end());
}

int estOcculte(Light l, Vec3 point, ArbreBinBoite* arbre, float eps) {
	Vec3 pointLum = l.position - point;
	Vec3 dirVersLum = pointLum.unitVector();
	float distancePointLum = pointLum.normSquared();
	Ray r{ point + dirVersLum * eps, dirVersLum };


	vector<Sphere> objetsScene = getSphereInBox(arbre, r);

	for (int i = 0; i < objetsScene.size(); i++)
	{
		optional<float> inter = objetsScene[i].intersect(r);

		if (inter.has_value() && inter.value() * inter.value() < distancePointLum)
			return 0;
	}

	return 1;
}

int main()
{
#pragma region definition Couleurs
	Vec3b red;
	red[0] = 0;
	red[1] = 0;
	red[2] = 255;

	Vec3b black;
	black[0] = 0;
	black[1] = 0;
	black[2] = 0;

	Vec3b white;
	white[0] = 255;
	white[1] = 255;
	white[2] = 255;

	Vec3b background;
	background = white;
#pragma endregion

	Timer clock;  // Timer ticks upon construction.

	int sizeX = 600;
	int sizeY = 600;

	Mat image = Mat::zeros(sizeX, sizeY, CV_8UC3);

	//Camera cam{ {sizeX / 2, sizeY / 2, -500} };
	Camera cam{ {50, -50, -500} };
	vector<Sphere> s;
	float rayon = 25;

	// generation des spheres
	for(int i = 0; i < 6; i++)
		for(int j = -10; j < 5; j++)
			for (int k = 0; k < 2; k++) 
				s.push_back({ {i * 2 * rayon + 1, j * 2 * rayon + 1, k * 2 * rayon + 1}, rayon, {1, 0, 0} });

	//Boite box{ {minX, minY, minZ}, {maxX, maxY, maxZ} };
	Boite box{ {0,0,0}, {0,0,0}};
	std::unordered_map<string, vector<Sphere>> m;

	float minX = 999999, minY = 999999, minZ = 999999;
	float maxX = -999999, maxY = -999999, maxZ = -999999;
	for (int i = 0; i < s.size(); i++) 
	{
		if (s[i].centre.x - rayon < minX)
			minX = s[i].centre.x - rayon;
		if (s[i].centre.x + rayon > maxX)
			maxX = s[i].centre.x + rayon;

		if (s[i].centre.y - rayon < minY)
			minY = s[i].centre.y - rayon;
		if (s[i].centre.y + rayon > maxY)
			maxY = s[i].centre.y + rayon;

		if (s[i].centre.z - rayon < minZ)
			minZ = s[i].centre.z - rayon;
		if (s[i].centre.z + rayon > maxZ)
			maxZ = s[i].centre.z + rayon;

		box.spheres.push_back(s[i]);
	}

	box.min = { minX, minY, minZ };
	box.max = { maxX, maxY, maxZ };

	int profondeurArbre = 3;
	ArbreBinBoite* racine = new ArbreBinBoite{ box };
	creerEnfants(racine, 3);
	//cout << minX << endl;
	//cout << racine->left->left->left->current.spheres.size() << endl;;

	//cout << minX << " " << maxX << endl;
	//cout << minY << " " << maxY << endl;
	//cout << minZ << " " << maxZ << endl;


	vector<Light> l;
	l.push_back({ {300, -100, -50}, 1200, { 0.5f, 0, 1} });

	for (int i = 0; i < sizeX; i++)
		for (int j = 0; j < sizeY; j++)
		{
			image.at<Vec3b>(j, i) = background;

			// Cam ortho
			Ray r{ cam.position + Vec3{ (float)(i - sizeX / 2), (float)(j - sizeY / 2), 0 }, {0, 0, 1} };


			vector<Sphere> sib = getSphereInBox(racine, r);
			//cout << sib.size() << endl;
			if (sib.size() > 0) {
				float inter = 9999999;
				bool intersect = false;
				int ks = 0;

				for (int k = 0; k < sib.size(); k++) {
					//optional<float> interK = s[box.spheres[k]].intersect(r);
					optional<float> interK = sib[k].intersect(r);

					if (interK.has_value() && interK.value() < inter) {
						inter = interK.value();
						ks = k;
						intersect = true;
					}
				}

				if (intersect)
				{
					Vec3 point = r.origin + inter * r.direction;
					Vec3 lum{ 0, 0, 0 };
					for (int a = 0; a < l.size(); a++)
					{
						//Vec3 lu = l[a].getLightOut(point, r.origin, s[ks].centre, s[ks].color);
						Vec3 lu = l[a].getLightOut(point, r.origin, sib[ks].centre, sib[ks].color);
						lum = lum + lu * estOcculte(l[a], point, s, 0.0006f);
						//lum = lum + lu * estOcculte(l[a], point, racine, 0.0006f);
					}

					Vec3b col;
					col[2] = clampColor(lum.x, 400);
					col[1] = clampColor(lum.y, 400);
					col[0] = clampColor(lum.z, 400);
					image.at<Vec3b>(j, i) = col;
				}
			}
				
		}

	clock.tock();
	std::cout << "Code run for " << clock.duration().count() << " ms" << std::endl;

	imshow("Display Window", image);
	waitKey(0);
	return 0;
};