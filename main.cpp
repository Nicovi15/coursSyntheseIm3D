#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "Vec3.h"
#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"
#include "Light.h"

using namespace cv;
using namespace std;

int clampColor(float x, float coef) {
	if ((int)(x/coef) > coef)
		return 255;
	else if ((int)x < 0)
		return 0;
	else
		return (int)(255 * (x / coef));
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
#pragma endregion

	int sizeX = 400;
	int sizeY = 400;

	Mat image = Mat::zeros(sizeX, sizeY, CV_8UC3);
	//Camera cam{ {-30, 0, 0}};
	//Sphere sMid { {0, 0, 101}, 75};
	//Sphere sRight{ {110, 0, 151}, 50 };
	//Sphere sLeft{ {-125, 0, 51}, 100 };
	//Sphere sBack{ {-30, 0, 0}, 300 };
	//
	////cout << "distance min : " + sqrt((Vec3{ 0,0,0 } -(s.centre - 50 * Vec3{0, 0, 1})).normSquared()) << endl;
	//
	//for (int i = 0; i < sizeX; i++)
	//	for (int j = 0; j < sizeY; j++) 
	//	{
	//		image.at<Vec3b>(j, i) = black;
	//
	//		Ray r{ cam.position + Vec3{ (float)(i - sizeX / 2), (float)(j - sizeY / 2), 0 }, {0, 0, 1}};
	//
	//		float interBack = sBack.intersect(r);
	//
	//		if (interBack > 0) {
	//			Vec3b col;
	//			col[0] = (int)(255 - (255 * interBack / 450.0f));
	//			col[1] = (int)(255 - (255 * interBack / 450.0f));
	//			col[2] = (int)(255 - (255 * interBack / 450.0f)); // < 0 ? 0 : (int)(255 - inter4 / 10);
	//			//col[2] = 0 + sqrt((Vec3{ 0,0,0 } -(s.centre - 50 * Vec3{ 0, 0, 1 })).normSquared()) + inter;
	//			image.at<Vec3b>(j, i) = col;
	//		}
	//
	//		float interRight = sRight.intersect(r);
	//
	//		if (interRight > 0) {
	//			Vec3b col;
	//			col[0] = 0;
	//			col[1] = 0;
	//			col[2] = (int)(255 - (255 * interRight / 151.0f)) < 0 ? 0 : (int)(255 - (255 * interRight / 151.0f));
	//			image.at<Vec3b>(j, i) = col;
	//		}
	//
	//		float interMid = sMid.intersect(r);
	//		if (interMid > 0) {
	//			Vec3b col;
	//			col[0] = 0;
	//			col[1] = 0;
	//			col[2] = (int)(255 - (255 * interMid / 151.0f));
	//			image.at<Vec3b>(j, i) = col;
	//		}
	//
	//		float interLeft = sLeft.intersect(r);
	//
	//		if (interLeft > 0) {
	//			Vec3b col;
	//			col[0] = 0;
	//			col[1] = 0;
	//			col[2] = (int)(255 - (255 * interLeft / 101.0f)) < 0 ? 255 : (int)(255 - (255 * interLeft / 101.0f));
	//			image.at<Vec3b>(j, i) = col;
	//		}
	//	}

	//imwrite("MyImage.png", image);

	Camera cam{ {0, 0, 0}};
	Sphere sol{ {0, 500, 0}, 500, {1, 1, 1} };
	Sphere murFond{ {0, 0, 800}, 500, {1, 1, 1} };
	Sphere murGauche{ {-660, 0, 0}, 500, {1, 1, 1} };
	Sphere murDroit{ {660, 0, 0}, 500, {1, 1, 1} };
	Sphere s1 { {-50, 20, 450}, 50, {1, 0, 0} };
	Sphere s2{ {100, 20, 300}, 50, {0, 0, 1} };
	Light l1{ {300, -300, 300}, 800, {1, 1, 1} };
	Light l2{ {-300, -300, -300}, 800, {1, 1, 1} };

	for (int i = 0; i < sizeX; i++)
		for (int j = 0; j < sizeY; j++)
		{
			image.at<Vec3b>(j, i) = black;

			Ray r{ cam.position + Vec3{ (float)(i - sizeX / 2), (float)(j - sizeY / 2), 0 }, {0, 0, 1} };

			
			float inter = murFond.intersect(r);
			if (inter > 0)
			{
				Vec3 point = r.origin + inter * r.direction;
				//std::cout << point.x << " " << point.y << " " << point.z << endl;
				//float lumO = l1.getLightOut(point, r.origin, murFond.centre);
				////std::cout << lumO << endl;
				//Vec3b col;
				//col[0] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[1] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[2] = (int)lumO < 0 ? 0 : (int)lumO;
				//image.at<Vec3b>(j, i) = col;
				Vec3 lumO = l1.getLightOut(point, r.origin, murFond.centre, murFond.color);
				Vec3 lum1 = l2.getLightOut(point, r.origin, murFond.centre, murFond.color);
				Vec3b col;
				//std::cout << lumO.x << endl;
				col[2] = clampColor(lumO.x + lum1.x, 400);//(int)lumO.x < 0 ? 0 : (int)lumO.x;
				col[1] = clampColor(lumO.y + lum1.y, 400);//(int)lumO.y < 0 ? 0 : (int)lumO.y;
				col[0] = clampColor(lumO.z + lum1.z, 400);//(int)lumO.z < 0 ? 0 : (int)lumO.z;
				image.at<Vec3b>(j, i) = col;

				//cout << lumO << endl;
			}

			inter = sol.intersect(r);
			if (inter > 0)
			{
				Vec3 point = r.origin + inter * r.direction;
				//std::cout << point.x << " " << point.y << " " << point.z << endl;
				//float lumO = l.getLightOut(point, r.origin, sol.centre);
				//std::cout << lumO << endl;
				//Vec3b col;
				//col[0] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[1] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[2] = (int)lumO < 0 ? 0 : (int)lumO;
				//image.at<Vec3b>(j, i) = col;
				//Vec3 lumO = l1.getLightOut(point, r.origin, sol.centre, sol.color);
				//Vec3b col;
				////std::cout << lumO.x << endl;
				//col[2] = clampColor(lumO.x, 300);//(int)lumO.x < 0 ? 0 : (int)lumO.x;
				//col[1] = clampColor(lumO.y, 300);//(int)lumO.y < 0 ? 0 : (int)lumO.y;
				//col[0] = clampColor(lumO.z, 300);//(int)lumO.z < 0 ? 0 : (int)lumO.z;
				//image.at<Vec3b>(j, i) = col;
				Vec3 lumO = l1.getLightOut(point, r.origin, sol.centre, sol.color);
				Vec3 lum1 = l2.getLightOut(point, r.origin, sol.centre, sol.color);
				Vec3b col;
				//std::cout << lumO.x << endl;
				col[2] = clampColor(lumO.x + lum1.x, 400);//(int)lumO.x < 0 ? 0 : (int)lumO.x;
				col[1] = clampColor(lumO.y + lum1.y, 400);//(int)lumO.y < 0 ? 0 : (int)lumO.y;
				col[0] = clampColor(lumO.z + lum1.z, 400);//(int)lumO.z < 0 ? 0 : (int)lumO.z;
				image.at<Vec3b>(j, i) = col;
				//cout << lumO << endl;
			}

			inter = murGauche.intersect(r);
			if (inter > 0)
			{
				Vec3 point = r.origin + inter * r.direction;
				//std::cout << point.x << " " << point.y << " " << point.z << endl;
				//float lumO = l1.getLightOut(point, r.origin, murGauche.centre);
				////std::cout << lumO << endl;
				//Vec3b col;
				//col[0] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[1] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[2] = (int)lumO < 0 ? 0 : (int)lumO;
				//image.at<Vec3b>(j, i) = col;
				Vec3 lumO = l1.getLightOut(point, r.origin, murGauche.centre, murGauche.color);
				Vec3 lum1 = l2.getLightOut(point, r.origin, murGauche.centre, murGauche.color);
				Vec3b col;
				//std::cout << lumO.x << endl;
				col[2] = clampColor(lumO.x + lum1.x, 400);//(int)lumO.x < 0 ? 0 : (int)lumO.x;
				col[1] = clampColor(lumO.y + lum1.y, 400);//(int)lumO.y < 0 ? 0 : (int)lumO.y;
				col[0] = clampColor(lumO.z + lum1.z, 400);//(int)lumO.z < 0 ? 0 : (int)lumO.z;
				image.at<Vec3b>(j, i) = col;
				//cout << lumO << endl;
			}

			inter = murDroit.intersect(r);
			if (inter > 0)
			{
				Vec3 point = r.origin + inter * r.direction;
				//std::cout << point.x << " " << point.y << " " << point.z << endl;
				//float lumO = l1.getLightOut(point, r.origin, murDroit.centre);
				////std::cout << lumO << endl;
				//Vec3b col;
				//col[0] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[1] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[2] = (int)lumO < 0 ? 0 : (int)lumO;
				//image.at<Vec3b>(j, i) = col;
				Vec3 lumO = l1.getLightOut(point, r.origin, murDroit.centre, murDroit.color);
				Vec3 lum1 = l2.getLightOut(point, r.origin, murDroit.centre, murDroit.color);
				Vec3b col;
				//std::cout << lumO.x << endl;
				col[2] = clampColor(lumO.x + lum1.x, 400);//(int)lumO.x < 0 ? 0 : (int)lumO.x;
				col[1] = clampColor(lumO.y + lum1.y, 400);//(int)lumO.y < 0 ? 0 : (int)lumO.y;
				col[0] = clampColor(lumO.z + lum1.z, 400);//(int)lumO.z < 0 ? 0 : (int)lumO.z;
				image.at<Vec3b>(j, i) = col;

				//cout << lumO << endl;
			}

			inter = s1.intersect(r);
			if (inter > 0) 
			{
				Vec3 point = r.origin + inter * r.direction;
				//std::cout << point.x << " " << point.y << " " << point.z << endl;
				//float lumO = l.getLightOut(point, r.origin, s1.centre);
				////std::cout << lumO << endl;
				//Vec3b col;
				//std::cout << lumO << endl;
				//col[0] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[1] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[2] = (int)lumO < 0 ? 0 : (int)lumO;
				Vec3 lumO = l1.getLightOut(point, r.origin, s1.centre, s1.color);
				Vec3 lum1 = l2.getLightOut(point, r.origin, s1.centre, s1.color);
				Vec3b col;
				//std::cout << lumO.x << endl;
				col[2] = clampColor(lumO.x + lum1.x, 400);//(int)lumO.x < 0 ? 0 : (int)lumO.x;
				col[1] = clampColor(lumO.y + lum1.y, 400);//(int)lumO.y < 0 ? 0 : (int)lumO.y;
				col[0] = clampColor(lumO.z + lum1.z, 400);//(int)lumO.z < 0 ? 0 : (int)lumO.z;
				image.at<Vec3b>(j, i) = col;

				//cout << lumO << endl;
			}

			inter = s2.intersect(r);
			if (inter > 0)
			{
				Vec3 point = r.origin + inter * r.direction;
				//std::cout << point.x << " " << point.y << " " << point.z << endl;
				//float lumO = l.getLightOut(point, r.origin, s2.centre);
				//std::cout << lumO << endl;
				//Vec3b col;
				//col[0] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[1] = (int)lumO < 0 ? 0 : (int)lumO;
				//col[2] = (int)lumO < 0 ? 0 : (int)lumO;

				Vec3 lumO = l1.getLightOut(point, r.origin, s2.centre, s2.color);
				Vec3 lum1 = l2.getLightOut(point, r.origin, s2.centre, s2.color);
				Vec3b col;
				//std::cout << lumO.z + lum1.z << endl;
				col[2] = clampColor(lumO.x + lum1.x, 400);//(int)lumO.x < 0 ? 0 : (int)lumO.x;
				col[1] = clampColor(lumO.y + lum1.y, 400);//(int)lumO.y < 0 ? 0 : (int)lumO.y;
				col[0] = clampColor(lumO.z + lum1.z, 400);//(int)lumO.z < 0 ? 0 : (int)lumO.z;

				image.at<Vec3b>(j, i) = col;

				//cout << lumO << endl;
			}
			
			
		}


	imshow("Display Window", image);
	waitKey(0);
	return 0;

}