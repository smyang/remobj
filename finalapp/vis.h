#include "bundleout.hpp"

void get_least_square_plane(Position* pos, int num_points, double* a, double* b, double* d);
void scene_point_to_image_point(Position* point_3d, Camera* cam, double* x, double* y);
void image_point_to_scene_point(double x, double y, Position* point_3d, Camera* cam, Plane p);
// returns mid2
int interpolate(int start1, int end1, int mid1, int start2, int end2);

struct PointPair{
	POINTS leftim_point;
	POINTS rightim_point;
};