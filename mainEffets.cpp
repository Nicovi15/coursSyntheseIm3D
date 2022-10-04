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
#include <string>

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

int estOcculte(Light l, Vec3 point, vector<Sphere> &objetsScene, float eps) {
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

	bool intersect(const Ray& r) const
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
	ArbreBinBoite* left;
	ArbreBinBoite* right;

};

void creerEnfants(ArbreBinBoite* pere, int n)
{
	if (n == 0)
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

vector<Sphere> getSphereInBox(const ArbreBinBoite* arbre, Ray r)
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
}

int estOcculte(Light l, Vec3 point, const ArbreBinBoite* arbre, float eps) {
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

std::optional<Vec3> getColorFromRay(Ray ray, const ArbreBinBoite* scene, vector<Light> lights, int profondeur) {

	if (profondeur < 0)
		return std::nullopt;

	vector<Sphere> sib = getSphereInBox(scene, ray);
	if (sib.size() > 0) {

		float inter = 9999999;
		bool intersect = false;
		int ks = 0;

		for (int k = 0; k < sib.size(); k++) {
			optional<float> interK = sib[k].intersect(ray);

			if (interK.has_value() && interK.value() < inter) {
				inter = interK.value();
				ks = k;
				intersect = true;
			}
		}

		if (intersect)
		{
			if (sib[ks].tag == "reflect")
			{
				Vec3 point = ray.origin + inter * ray.direction;
				Vec3 D = ray.direction.norm();
				Vec3 N = (point - sib[ks].centre).norm();
				Vec3 R = D - (2 * (N.dot(D)) * N);
				Ray newRay{ point + R * 0.0006f, R};
				Vec3 lum{ 0, 0, 0 };

				std::optional<Vec3> refColor = getColorFromRay(newRay, scene, lights, profondeur - 1);
				if (refColor.has_value())
					lum = lum + refColor.value();

				return lum;
			}
			else if (sib[ks].tag == "refract") {

				// TODO : ajout du calcul du reflet et faire la somme des deux couleurs lum = 0.5 * refract + 0.5 * reflect
				// ou tirer un random et selon le résultat tirer soit un refract, soit un reflect
				float eta = 1.5f;
				Vec3 point = ray.origin + inter * ray.direction;
				Vec3 D = ray.direction.norm();
				Vec3 N = (point - sib[ks].centre).norm();
				float k = 1.0f - eta * eta * (1.0f - N.dot(D) * N.dot(D));
				if(k < 0){
					return std::nullopt;
				}
				else {
					Vec3 R = eta * D - eta * (N.dot(D) + sqrt(k)) * N;
					Ray newRay{ point + R * 0.0006f, R };
					Vec3 lum{ 0, 0, 0 };

					std::optional<Vec3> refColor = getColorFromRay(newRay, scene, lights, profondeur - 1);
					if (refColor.has_value())
						lum = lum + refColor.value();

					return lum;
				}
			}
			else
			{
				Vec3 point = ray.origin + inter * ray.direction;
				Vec3 lum{ 0, 0, 0 };
				for (int a = 0; a < lights.size(); a++)
				{
					Vec3 lu = lights[a].getLightOut(point, ray.origin, sib[ks].centre, sib[ks].color);
					lum = lum + lu * estOcculte(lights[a], point, scene, 0.0006f);
				}
				return lum;
			}
		}
		else 
			return std::nullopt;
	}
	else
		return std::nullopt;
}

int main(){
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
	Camera cam{ {0, -100, -500} };
	vector<Sphere> s;
	float rayon = 25;

	// generation des spheres
	//for (int i = 0; i < 3; i++)
	//	for (int j = 0; j < 5; j++)
	//		for (int k = 0; k < 1; k++)
	//			s.push_back({ {i * 2 * rayon + 1, j * 2 * rayon + 1, k * 2 * rayon + 1}, rayon, {1, 0, 0} });
	s.push_back({ {0, 0, 550}, 500, {1, 1, 1}, "" });		 // fond
	s.push_back({ {0, 500, 100}, 500, {1, 1, 1}, "" });		 // sol
	s.push_back({ {-920, -60, 0}, 700, {1, 1, 1}, "" });	 // mur gauche
	s.push_back({ {920, -60, 0}, 700, {1, 1, 1}, "" });		 // mur droit

	//s.push_back({ {-75, -60, 0}, 70, {1, 0, 0}, false });		 // sphere rouge
	//s.push_back({ {90, -300, 0}, 70, {0, 0, 1}, false });		 // sphere bleue
	//s.push_back({ {75, -60, 0}, 70, {1, 1, 1}, true });		 // sphere mirroir
	s.push_back({ {-75, -60, 0}, 70, {1, 0, 0}, "reflect" });
	s.push_back({ {75, -60, 0}, 70, {1, 0, 0}, "refract" });
	s.push_back({ {0, -200, 0}, 70, {0, 1, 0}, "" });

	Boite box{ {0,0,0}, {0,0,0} };
	std::unordered_map<string, vector<Sphere>> m;

	float minX = 999999, minY = 999999, minZ = 999999;
	float maxX = -999999, maxY = -999999, maxZ = -999999;
	for (int i = 0; i < s.size(); i++)
	{
		if (s[i].centre.x - s[i].radius < minX)
			minX = s[i].centre.x - s[i].radius;
		if (s[i].centre.x + s[i].radius > maxX)
			maxX = s[i].centre.x + s[i].radius;

		if (s[i].centre.y - s[i].radius < minY)
			minY = s[i].centre.y - s[i].radius;
		if (s[i].centre.y + s[i].radius > maxY)
			maxY = s[i].centre.y + s[i].radius;

		if (s[i].centre.z - s[i].radius < minZ)
			minZ = s[i].centre.z - s[i].radius;
		if (s[i].centre.z + s[i].radius > maxZ)
			maxZ = s[i].centre.z + s[i].radius;

		box.spheres.push_back(s[i]);
	}

	box.min = { minX, minY, minZ };
	box.max = { maxX, maxY, maxZ };

	int profondeurArbre = 1;
	ArbreBinBoite* racine = new ArbreBinBoite{ box };
	creerEnfants(racine, profondeurArbre);


	vector<Light> l;
	//l.push_back({ {0, -600, 0}, 1200, { 0.5f, 0, 1} });
	l.push_back({ {0, -200, -300}, 1200, { 1, 1, 1} });
	//l.push_back({ {300, 0, -300}, 1200, { 0.5f, 0, 1} });

	for (int i = 0; i < sizeX; i++)
		for (int j = 0; j < sizeY; j++)
		{
			//image.at<Vec3b>(j, i) = background;

			// Cam ortho
			Ray r{ cam.position + Vec3{ (float)(i - sizeX / 2), (float)(j - sizeY / 2), 0 }, {0, 0, 1} };

			//Vec3 posPixel = cam.position + Vec3{ (float)(i - sizeX / 2), (float)(j - sizeY / 2), 0 };
			//Ray r{ posPixel, (posPixel - cam.position + Vec3{0, 0, -10}).unitVector() 

			//Vec3 posPixel = Vec3{ 0,0,0 } +Vec3{ (float)(i - sizeX / 2), (float)(j - sizeY / 2), 0 };
			//Ray r{ posPixel, (posPixel - Vec3{ 0,0,0 } + Vec3{0, 0, -5}).unitVector() };

			
			std:optional<Vec3> pixelColor = getColorFromRay(r, racine, l, 10);
			if (pixelColor.has_value()) {
				Vec3b col;
				col[2] = clampColor(pixelColor.value().x, 400);
				col[1] = clampColor(pixelColor.value().y, 400);
				col[0] = clampColor(pixelColor.value().z, 400);
				image.at<Vec3b>(j, i) = col;
			}
			else 
				image.at<Vec3b>(j, i) = background;
				
			
			//
			//cout << sib.size() << endl;
			
			
			//
			//vector<Sphere> sib = s;
			/*
			vector<Sphere> sib = getSphereInBox(racine, r);
			if (sib.size() > 0) {
				float inter = 9999999;
				bool intersect = false;
				int ks = 0;

				for (int k = 0; k < sib.size(); k++) {
					optional<float> interK = sib[k].intersect(r);

					if (interK.has_value() && interK.value() < inter) {
						inter = interK.value();
						ks = k;
						intersect = true;
					}
				}

				if (intersect)
				{
					if (sib[ks].miroir) 
					{
						Vec3 point = r.origin + inter * r.direction;
						Vec3 D = r.direction.norm();
						Vec3 N = (point - sib[ks].centre).norm();
						Vec3 R = D - (2 * (N.dot(D)) * N);
						Ray newR{ point, R};
						vector<Sphere> sib2 = getSphereInBox(racine, newR);
					
						if (sib2.size() > 0) {
							float inter2 = 9999999;
							bool intersect2 = false;
							int ks2 = 0;

							for (int k2 = 0; k2 < sib2.size(); k2++) {
								//if (sib2[k2].color.x == sib[ks].color.x && sib2[k2].color.y == sib[ks].color.y && sib2[k2].color.z == sib[ks].color.z)
								//	continue;
								optional<float> interK2 = sib2[k2].intersect(newR);

								if (interK2.has_value() && interK2.value() < inter2) {
									inter2 = interK2.value();
									ks2 = k2;
									intersect2 = true;
								}
							}

							if (intersect2)
							{
								Vec3 point2 = newR.origin + inter2 * newR.direction;
								Vec3 lum{ 0, 0, 0 };
								for (int a = 0; a < l.size(); a++)
								{
									//cout << sib[ks2].color.x << " " << sib[ks2].color.y << " " << sib[ks2].color.z << endl;
									Vec3 lu = l[a].getLightOut(point2, newR.origin, sib[ks2].centre, sib[ks2].color);
									lum = lum + lu * estOcculte(l[a], point2, s, 0.0006f);
									Vec3 lu2 = l[a].getLightOut(point, r.origin, sib[ks].centre, sib[ks].color);
									lum = lum + lu2 * estOcculte(l[a], point, s, 0.0006f);
								}

								Vec3b col;
								col[2] = clampColor(lum.x, 400);
								col[1] = clampColor(lum.y, 400);
								col[0] = clampColor(lum.z, 400);
								image.at<Vec3b>(j, i) = col;
							}
							else {
								Vec3 point = r.origin + inter * r.direction;
								Vec3 lum{ 0, 0, 0 };
								for (int a = 0; a < l.size(); a++)
								{
									Vec3 lu = l[a].getLightOut(point, r.origin, sib[ks].centre, sib[ks].color);
									lum = lum + lu * estOcculte(l[a], point, s, 0.0006f);
								}
							
								Vec3b col;
								col[2] = clampColor(lum.x, 400);
								col[1] = clampColor(lum.y, 400);
								col[0] = clampColor(lum.z, 400);
								col[2] = 0;
								col[1] = 0;
								col[0] = 0;
								image.at<Vec3b>(j, i) = col;
							}
						}
						else {
							cout << "hey" << endl;
							Vec3 point = r.origin + inter * r.direction;
							Vec3 lum{ 0, 0, 0 };
							for (int a = 0; a < l.size(); a++)
							{
								Vec3 lu = l[a].getLightOut(point, r.origin, sib[ks].centre, sib[ks].color);
								lum = lum + lu * estOcculte(l[a], point, s, 0.0006f);
							}

							Vec3b col;
							col[2] = clampColor(lum.x, 400);
							col[1] = clampColor(lum.y, 400);
							col[0] = clampColor(lum.z, 400);
							image.at<Vec3b>(j, i) = col;
						}
					
					}
					else 
					{
						Vec3 point = r.origin + inter * r.direction;
						Vec3 lum{ 0, 0, 0 };
						for (int a = 0; a < l.size(); a++)
						{
							Vec3 lu = l[a].getLightOut(point, r.origin, sib[ks].centre, sib[ks].color);
							//lum = lum + lu * estOcculte(l[a], point, s, 0.0006f);
							lum = lum + lu * estOcculte(l[a], point, racine, 0.0006f);
						}

						Vec3b col;
						col[2] = clampColor(lum.x, 400);
						col[1] = clampColor(lum.y, 400);
						col[0] = clampColor(lum.z, 400);
						image.at<Vec3b>(j, i) = col;
					}
					
				}
			}
			*/
			
			
		}

	clock.tock();
	std::cout << "Code run for " << clock.duration().count() << " ms" << std::endl;

	imshow("Display Window", image);
	waitKey(0);
	return 0;
};