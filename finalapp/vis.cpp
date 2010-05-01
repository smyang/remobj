// vis.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "vision_utilities.hpp"
#include "bundleout.hpp"
#include <math.h>
#include "../taucs/taucsaddon.h"
#include "matrix.h"
#include "vis.h"


void copy_image(ImageColor* imgsrc, ImageColor* imgdest)
{
    int row = getNRowsColor(imgsrc);
    int col = getNColsColor(imgsrc);	

	for (int i=0;i<row;i++) {
		for (int j=0;j<col;j++) {
			setPixelColor(imgdest, i, j, getPixelColor(imgsrc, i, j, 1), getPixelColor(imgsrc, i, j, 2), getPixelColor(imgsrc, i, j, 3));
		}
	}
}

double mult_f_and_r(double px, double py, double f, double k1, double k2, double val)
{
	return f*(1.0f+k1*(px*px+py*py)+k2*(px*px+py*py)*(px*px+py*py))*val;
}

void image_point_to_scene_point(double x, double y, Position* point_3d, Camera* cam, Plane p)
{
	if (x==0) x=1; // XXX: a big workaround to divide by zero

	// TODO: handle divide by zero case
	// p' = f * r(p) * p
	double left = 0;
	double right = x;
	double px = x/2;
	double py = y/2;

	int counter =0;

	/*if (x==0) {
		left = 0;
		right = y;
		while (1) {
			if (counter++ >= 100) { // quit and error if more than 100 iterations
				fprintf(stderr, "image_point_to_Scene_point: error in iterative method\n");
				return;
			}

			double estimate_y = mult_f_and_r(px, py, cam->f, cam->k1, cam->k2, py);
			if (abs(y-estimate_y<0.0000001))
				break;
			if (abs(estimate_y) > abs(y)) {
				right = py;
				py = (left+right)/2;
			}else {
				left = py;
				py = (left+right)/2;
			}
		}*/
	//}else {
		while (1) {
			if (counter++ >= 100) { // quit and error if more than 100 iterations
				fprintf(stderr, "image_point_to_Scene_point: error in iterative method\n");
				return;
			}
			double estimate_x = mult_f_and_r(px, py, cam->f, cam->k1, cam->k2, px);
			double estimate_y = mult_f_and_r(px, py, cam->f, cam->k1, cam->k2, py);
			if (abs(x-estimate_x)<0.0000001) { // px, py are good enough
				break;
			}
			if (abs(estimate_x) > abs(x)) {
				right = px;
				px = (left+right)/2;
				py = px * y / x;
			}else {
				left = px;
				px = (left+right)/2;
				py = px * y / x;
			}
		}
	//}

	//printf("%lf %lf\n", px, py);

	double pz = 1;

	// p = -P / P.z
	// P = R * X + t
	// X = [ X Y Z ] is our target point in 3d (unknown)
	// R is rotation matrix, t is translation vector
	// p is [px py pz]'
	// so R11*X+R12*Y+R13*Z+tx=Px
	//    R21*X+R22*Y+R23*Z+ty=Py
	//    R31*X+R32*Y+R33*Z+tz=Pz
	// and px = -Px / Pz
	//     py = -Py / Pz
	// plus plane constraint AX+BY+Z+D=0
	// solve the linear equations

	double rhs[3];
	// equation1 : (R11+px*R31)x+(R12+px*R32)y+(R13+px*R33)z+(t1+px*t3)=0

	// write to ijv file
	FILE* fp = fopen("3d_recover.ijv", "w");
	fprintf(fp, "1 1 %lf\n", cam->rotation[0][0]+px*cam->rotation[2][0]);
	fprintf(fp, "1 2 %lf\n", cam->rotation[0][1]+px*cam->rotation[2][1]);
	fprintf(fp, "1 3 %lf\n", cam->rotation[0][2]+px*cam->rotation[2][2]);
	rhs[0] = -cam->translation[0]-px*cam->translation[2];

	fprintf(fp, "2 1 %lf\n", cam->rotation[1][0]+py*cam->rotation[2][0]);
	fprintf(fp, "2 2 %lf\n", cam->rotation[1][1]+py*cam->rotation[2][1]);
	fprintf(fp, "2 3 %lf\n", cam->rotation[1][2]+py*cam->rotation[2][2]);
	rhs[1] = -cam->translation[1]-py*cam->translation[2];

	fprintf(fp, "3 1 %lf\n", p.a);
	fprintf(fp, "3 2 %lf\n", p.b);
	fprintf(fp, "3 3 %lf\n", 1.0f);
	rhs[2] = -p.d;
	fclose(fp);
	
	taucs_ccs_matrix *pB = taucs_ccs_read_ijv("3d_recover.ijv", TAUCS_DOUBLE);

	double result[3];

	// Run the solver, "LU" means no symmetry is required
	char* options[] = { "taucs.factor.LU=true", NULL };
	if (taucs_linsolve(pB, NULL, 1, result, rhs, options, NULL) != TAUCS_SUCCESS) {
		printf("Solver failed\n");
	}
	printf("result %lf %lf %lf\n", result[0], result[1], result[2]);
	point_3d->x = result[0];
	point_3d->y = result[1];
	point_3d->z = result[2];
	taucs_free(pB);

	/*
	double test_x = 0.901753f, test_y=-0.323782f, test_z=-4.808777f;
	//double test_x = result[0], test_y=result[1], test_z=result[2];
	double myp3 = cam->rotation[2][0]*test_x+cam->rotation[2][1]*test_y+cam->rotation[2][2]*test_z+cam->translation[2];
	double myp1 = cam->rotation[0][0]*test_x+cam->rotation[0][1]*test_y+cam->rotation[0][2]*test_z+cam->translation[0];
	printf("%lf\n", -myp1/myp3);
	printf("%lf \n", myp3*px+myp1);

	printf("eq1 %lf\n", (cam->rotation[0][0]+px*cam->rotation[2][0])*test_x +
		(cam->rotation[0][1]+px*cam->rotation[2][1])*test_y +
		(cam->rotation[0][2]+px*cam->rotation[2][2])*test_z - rhs[0]);

	printf("eq2 %lf\n", (cam->rotation[1][0]+py*cam->rotation[2][0])*test_x +
		(cam->rotation[1][1]+py*cam->rotation[2][1])*test_y +
		(cam->rotation[1][2]+py*cam->rotation[2][2])*test_z - rhs[1]);

	printf("eq3 %lf\n", p.a*test_x+p.b*test_y+test_z+p.d);
	*/
}

void scene_point_to_image_point(Position* point_3d, Camera* cam, double* x, double* y)
{
	/////////////////////
	// map from 3D point to 2D image

	// R: rotation matrix
	math::matrix<double> R;
	for (int i=0;i<3;i++)
		for (int j=0;j<3;j++)
			R(i,j) = cam->rotation[i][j];
	R.SetSize(3,3);

	// X: the 3d point
	math::matrix<double> X;
	X(0,0) = point_3d->x;
	X(1,0) = point_3d->y;
	X(2,0) = point_3d->z;
	X.SetSize(3,1);

	math::matrix<double> t;
	t(0,0) = cam->translation[0];
	t(1,0) = cam->translation[1];
	t(2,0) = cam->translation[2];
	t.SetSize(3,1);

	math::matrix<double> P;
	P = R *  X + t;

	math::matrix<double> p;
	p = -P / P(2,0);

	double norm = p.Norm();
	double rp = 1.0f + cam->k1*norm*norm + cam->k2*norm*norm*norm*norm;

	math::matrix<double> p2;
	p2 = cam->f*rp*p;

	*x = p2(0,0);
	*y = p2(1,0);

	//printf("p: %lf %lf %lf\n", p(0,0), p(1,0), p(2,0));
	//printf("p2: %lf %lf %lf\n", p2(0,0), p2(1,0), p2(2,0));
}

void big_dot(ImageColor* img, int r, int c)
{
	setPixelColor(img, r,   c,   255, 0, 0);
	setPixelColor(img, r+1, c,   255, 0, 0);
	setPixelColor(img, r,   c+1, 255, 0, 0);
	setPixelColor(img, r-1, c,   255, 0, 0);
	setPixelColor(img, r,   c-1, 255, 0, 0);
}

void get_least_square_plane(Position* pos, int num_points, double* a, double* b, double* d)
{
	// three equations:
	// sum((ax_i+by_i+z_i+d)*x_i) = 0;  a*p1 + b*p2 + p3 + d*p4 = 0
	// sum((ax_i+by_i+z_i+d)*y_i) = 0;  a*q1 + b*q2 + q3 + d*q4 = 0
	// sum((ax_i+by_i+z_i+d)) = 0;      a*r1 + b*r2 + r3 + d*r4 = 0

	double p1=0, p2=0, p3=0, p4=0;
	double q1=0, q2=0, q3=0, q4=0;
	double r1=0, r2=0, r3=0, r4=0;

	for (int i=0;i<num_points;i++) {
		double x_i = pos[i].x;
		double y_i = pos[i].y;
		double z_i = pos[i].z;
		p1 += x_i*x_i;
		p2 += y_i*x_i;
		p3 += z_i*x_i;
		p4 += x_i;
		q1 += x_i*y_i;
		q2 += y_i*y_i;
		q3 += z_i*y_i;
		q4 += y_i;
		r1 += x_i;
		r2 += y_i;
		r3 += z_i;
		r4 += 1;
	}

	// write to ijv file
	FILE* fp = fopen("least_square.ijv", "w");
	fprintf(fp, "1 1 %lf\n", p1);
	fprintf(fp, "1 2 %lf\n", p2);
	fprintf(fp, "1 3 %lf\n", p4);
	fprintf(fp, "2 1 %lf\n", q1);
	fprintf(fp, "2 2 %lf\n", q2);
	fprintf(fp, "2 3 %lf\n", q4);
	fprintf(fp, "3 1 %lf\n", r1);
	fprintf(fp, "3 2 %lf\n", r2);
	fprintf(fp, "3 3 %lf\n", r4);
	fclose(fp);
	
	taucs_ccs_matrix *pB = taucs_ccs_read_ijv("least_square.ijv", TAUCS_DOUBLE);

	double rhs[3] = {-p3, -q3, -r3};
	double result[3];

	// Run the solver, "LU" means no symmetry is required
	char* options[] = { "taucs.factor.LU=true", NULL };
	if (taucs_linsolve(pB, NULL, 1, result, rhs, options, NULL) != TAUCS_SUCCESS) {
		printf("Solver failed\n");
	}
	printf("least square plane %lf %lf %lf\n", result[0], result[1], result[2]);
	*a = result[0];
	*b = result[1];
	*d = result[2];
	taucs_free(pB);

	double sum=0;
	double var=0;
	for (int i=0;i<num_points;i++) {
		double x_i = pos[i].x;
		double y_i = pos[i].y;
		double z_i = pos[i].z;
		double diff = x_i**a+y_i**b+z_i+*d;
		sum += abs(diff);
		var += diff*diff;
	}
	double avg = sum/num_points;
	var = var;
	double std = sqrt(var);

	int counter=0;
	for (int i=0;i<num_points;i++) {
		double x_i = pos[i].x;
		double y_i = pos[i].y;
		double z_i = pos[i].z;
		double diff = x_i**a+y_i**b+z_i+*d;
		if ((abs(diff)) > avg)
			counter++;
	}
}

int interpolate(int start1, int end1, int mid1, int start2, int end2)
{
	if (end1==start1) return start2;
	return (int)(start2 + (end2-start2)*(mid1-start1)/(double)(end1-start1));
}


#if 0
int _tmain(int argc, _TCHAR* argv[])
{
	// parse input parameters
	// 1st set
	/*
    const char *inputFilename1 = "IMG_0437.ppm";
	const char *inputFilename2 = "IMG_0438.ppm";
    const char *outputFilename1 = "IMG_0437_out.ppm";
	const char *outputFilename2 = "IMG_0438_out.ppm";
	*/

    const char *inputFilename1 = "IMG_1149.ppm";
	const char *inputFilename2 = "IMG_1150.ppm";
    const char *outputFilename1 = "IMG_1149_out.ppm";
	const char *outputFilename2 = "IMG_1150_out.ppm";

	char *bundle_out_filename = "bundle.sas3.out";

    ImageColor img1, img2, imgout1, imgout2;

    if (readImageColor(&img1, inputFilename1)==0)
        printf("Gray-scale image \"%s\" properly read\n", inputFilename1);
    else {
        printf("Error reading binary image \"%s\"\n", inputFilename1);
        return -1;
    }

    if (readImageColor(&img2, inputFilename2)==0)
        printf("Gray-scale image \"%s\" properly read\n", inputFilename2);
    else {
        printf("Error reading binary image \"%s\"\n", inputFilename2);
        return -1;
    }

    int row1 = getNRowsColor(&img1);
    int col1 = getNColsColor(&img1);
	int row2 = getNRowsColor(&img2);
    int col2 = getNColsColor(&img2);

	setSizeColor(&imgout1, row1, col1*2);
	setColorsColor(&imgout1, 255);
	setColorsColor(&imgout2, 255);

	copy_image(&img1, &imgout1);
	//copy_image(&img2, &imgout2);

	for (int i=0;i<row1;i++)
		for (int j=0;j<col1;j++)
			setPixelColor(&imgout1, i, col1+j, getPixelColor(&img2, i, j, 1), getPixelColor(&img2, i, j, 2), getPixelColor(&img2, i, j, 3));

	big_dot(&imgout1, -142, 92);
	//big_dot(&imgout2, -84, 29);

	BundleOut bundle_out;
	bundle_out.parse_file(bundle_out_filename);


	int x[] = {217,367,207,368};
	int y[] = {289,293,442,450};
	int c_x = col1/2;
	int c_y = row1/2;

	printf("num %d\n", bundle_out.num_points);

	for (int i=0;i<bundle_out.num_points;i++) {
		//for (int j=0;j<4;j++) {
		//	int x_diff = x[j]-c_x-bundle_out.points[i][0];
		//	int y_diff = y[j]-c_y+bundle_out.points[i][1];
		//	if (sqrt((float)(x_diff*x_diff+y_diff*y_diff)) < 50) {
				//printf("y=%d x=%d, y=%d x=%d, i=%d, j=%d\n", c_y-bundle_out.points[i][1], bundle_out.points[i][0]+c_x, c_y-bundle_out.points[i][3], bundle_out.points[i][2]+c_x, i, j);
	//			big_dot(&imgout1, c_y-bundle_out.points[i][1], bundle_out.points[i][0]+c_x);
	//			big_dot(&imgout1, c_y-bundle_out.points[i][3], bundle_out.points[i][2]+c_x+col1);
		//	}
		//}
	}

	// get least square plane
	double a,b,d;
	get_least_square_plane(bundle_out.points_3d, bundle_out.num_points, &a, &b, &d);

	double myx,myy;
	printf("3d test point %lf %lf %lf\n", bundle_out.points_3d[0], bundle_out.points_3d[1], bundle_out.points_3d[2]);
	scene_point_to_image_point(&bundle_out.points_3d[0], &bundle_out.camera[0], &myx, &myy);
	Plane plane;
	plane.a = a;
	plane.b = b;
	plane.d = d;
	//plane.a = 3.0f;
	//plane.b = 5.0f;
	//plane.d = 3.722428;
	Position pos;
	//image_point_to_scene_point(myx, myy, &pos, &bundle_out.camera[0], plane);
	image_point_to_scene_point(bundle_out.points[0][0], bundle_out.points[0][1], &pos, &bundle_out.camera[0], plane);

	double myx2, myy2;
	scene_point_to_image_point(&pos, &bundle_out.camera[1], &myx2, &myy2);

	big_dot(&imgout1, c_y-bundle_out.points[0][1], c_x+bundle_out.points[0][0]);
	big_dot(&imgout1, c_y-myy2, c_x+col1+myx2);

	printf("image1: %d %d\n", bundle_out.points[0][0], bundle_out.points[0][1]);
	printf("image2: bundler(%d %d) reconstruct(%lf %lf)\n", bundle_out.points[0][2], bundle_out.points[0][3], myx2, myy2);

	// draw feature points matched by bundler
	/*for (int i=0;i<bundle_out.num_points;i++) {
		if (bundle_out.points_3d[i].z + 19 > 3) 
			big_dot(&imgout1, c_y-bundle_out.points[i][1], bundle_out.points[i][0]+c_x);
	}*/

#ifdef BLEND
	// right triangle
	int pair1[4] = {245,234,229,148}; // row1, col1, row2, col2, CV notation
	int pair2[4] = {250,367,234,282};
	//int pair3[4] = {302,228,285,147};
	int pair3[4] = {477,205,477,223};


    // triangle
	//
	//            |\ vec_right_up
	//  vec_left  | \  
	//            | /
	//            |/  vec_right_down

	int vec1_left[2]  = {pair3[0]-pair1[0], pair3[1]-pair1[1]}; // the left vector of image1
	int vec1_right_up[2] = {pair2[0]-pair1[0], pair2[1]-pair1[1]}; // the upper right vector of image1
	int vec1_right_down[2] = {pair3[0]-pair2[0], pair3[1]-pair2[1]}; // the lower right vector of image1

	int vec2_left[2] = {pair3[2]-pair1[2], pair3[3]-pair1[3]}; // the left vector of image2
	int vec2_right[2] = {pair2[2]-pair1[2], pair2[3]-pair1[3]}; // the right vector of image2

	// upper half
	for (int y=pair1[0]; y<=pair2[0];y++) {
		int left  = pair1[1] + (int)(vec1_left[1] *(y-pair1[0])/(double)(vec1_left[0]));
		int right = pair1[1] + (int)(vec1_right_up[1]*(y-pair1[0])/(double)(vec1_right_up[0]));
		for (int x=left; x<=right; x++) {

			// vector (p,q): from pair1 to current point
			int p = x-pair1[1];
			int q = y-pair1[0];

			// (p,q) can be represented as a*vec1_left+b*vec1_right
			int x1=vec1_left[1], y1=vec1_left[0];
			int x2=vec1_right_up[1], y2=vec1_right_up[0];
			double a,b;
			if (y1*x2-x1*y2==0) {
				printf("denominator is zero!\n");
				a=0;
				b=0;
			}else {
				a = (q*x2-p*y2)/(double)(y1*x2-x1*y2);
				b = (p*y1-q*x1)/(double)(y1*x2-x1*y2);
			}

			// find the point on second image
			int image2_y = (int)(pair1[2] + a*vec2_left[0] + b*vec2_right[0]);
			int image2_x = (int)(pair1[3] + a*vec2_left[1] + b*vec2_right[1]);
			setPixelColor(&imgout1, y, x, getPixelColor(&img2, image2_y, image2_x, 1),
				getPixelColor(&img2, image2_y, image2_x, 2), getPixelColor(&img2, image2_y, image2_x, 3));
			//printf("%d %d %d %d\n", y, x, image2_y, image2_x);
		}
		//lineColor(&imgout1, y, left, y, right, 255, 255, 255);
		//printf("%d %d\n", left, right);
	}

	for (int y=pair2[0]; y<=pair3[0];y++) {
		int left  = pair1[1] + (int)(vec1_left[1] *(y-pair1[0])/(double)(vec1_left[0]));
		int right = pair2[1] + (int)(vec1_right_down[1]*(y-pair2[0])/(double)(vec1_right_down[0]));
		for (int x=left; x<=right; x++) {

			// vector (p,q): from pair1 to current point
			int p = x-pair1[1];
			int q = y-pair1[0];

			// (p,q) can be represented as a*vec1_left+b*vec1_right
			int x1=vec1_left[1], y1=vec1_left[0];
			int x2=vec1_right_up[1], y2=vec1_right_up[0];
			double a,b;
			if (y1*x2-x1*y2==0) {
				printf("denominator is zero!\n");
				a=0;
				b=0;
			}else {
				a = (q*x2-p*y2)/(double)(y1*x2-x1*y2);
				b = (p*y1-q*x1)/(double)(y1*x2-x1*y2);
			}

			// find the point on second image
			int image2_y = (int)(pair1[2] + a*vec2_left[0] + b*vec2_right[0]);
			int image2_x = (int)(pair1[3] + a*vec2_left[1] + b*vec2_right[1]);
			setPixelColor(&imgout1, y, x, getPixelColor(&img2, image2_y, image2_x, 1),
				getPixelColor(&img2, image2_y, image2_x, 2), getPixelColor(&img2, image2_y, image2_x, 3));
			//printf("%d %d %d %d\n", y, x, image2_y, image2_x);
		}
	}
#endif //BLEND
	for (int i=0;i<bundle_out.num_points;i++) {
	//for (int i=20;i<40;i++) {
		//lineColor(&imgout1, row1/2-bundle_out.points[i][3], col1+col1/2+b.points[i][2], row1/2-bundle_out.points[i][1], col1/2+bundle_out.points[i][0], 255, 255, 255);
		//printf("%lf %lf %lf\n", bundle_out.points_3d[i].x, bundle_out.points_3d[i].y, bundle_out.points_3d[i].z);
	}
	printf("ok\n");


	if (writeImageColor(&imgout1, outputFilename1)==0 )
		printf("Gray-scale image %s written\n", outputFilename1);
    else 
		printf("Error writing gray-scale image %s\n", outputFilename1);


/*
	if (writeImageColor(&imgout2, outputFilename2)==0)
		printf("Gray-scale image %s written\n", outputFilename2);
    else 
		printf("Error writing gray-scale image %s\n", outputFilename2);
*/

	return 0;
}

#endif