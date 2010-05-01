#include "stdafx.h"

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "bundleout.hpp"

#define MAX_BUF 1024

BundleOut::BundleOut() : points(NULL),points_3d(NULL),camera(NULL)
{
}

BundleOut::~BundleOut()
{
	for (int i=0;i<num_points;i++) {
		delete [] points[i];
	}
	delete [] points;
	delete [] points_3d;
	delete [] camera;
}

// check the output format of 
void BundleOut::parse_file(char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "error reading file %s\n", filename);
		return;
	}

	char buf[MAX_BUF];
	fgets(buf, MAX_BUF-1, fp);
	fscanf(fp, "%d %d\n", &num_cameras, &num_points);

	// parsing camera
	camera = new Camera[num_cameras];
	for (int i=0;i<num_cameras;i++) {
		fscanf(fp, "%lf %lf %lf\n", &camera[i].f, &camera[i].k1, &camera[i].k2);
		fscanf(fp, "%lf %lf %lf\n", camera[i].rotation[0], camera[i].rotation[0]+1, camera[i].rotation[0]+2);
		fscanf(fp, "%lf %lf %lf\n", camera[i].rotation[1], camera[i].rotation[1]+1, camera[i].rotation[1]+2);
		fscanf(fp, "%lf %lf %lf\n", camera[i].rotation[2], camera[i].rotation[2]+1, camera[i].rotation[2]+2);
		fscanf(fp, "%lf %lf %lf\n", &camera[i].translation[0], &camera[i].translation[1], &camera[i].translation[2]);
	}

	points = new int*[num_points];
	points_3d = new Position[num_points];
	for (int i=0;i<num_points;i++)
		points[i] = NULL;

	for (int i=0;i<num_points;i++) {
		int r,g,b;
		int list_len, cam_index, key;
		double pointx, pointy;
		fscanf(fp, "%lf %lf %lf\n", &points_3d[i].x, &points_3d[i].y, &points_3d[i].z);
		fscanf(fp, "%d %d %d\n", &r, &g, &b);
		fscanf(fp, "%d", &list_len);
		if (list_len!=2) {
			printf("list length is not 2!!\n");
		} else {
			points[i] = new int[4];
			fscanf(fp, "%d %d %lf %lf", &cam_index, &key, &pointx, &pointy);
			points[i][0] = (int)pointx;
			points[i][1] = (int)pointy;
			fscanf(fp, "%d %d %lf %lf", &cam_index, &key, &pointx, &pointy);
			points[i][2] = (int)pointx;
			points[i][3] = (int)pointy;
		}
	}

	fclose(fp);	
}
