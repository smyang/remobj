#ifndef _FINALAPP_BUNDLEOUT_HPP_
#define _FINALAPP_BUNDLEOUT_HPP_
struct Position{
	double x,y,z;
	Position() {}
	Position(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }
};

struct Plane {
	// Ax+By+z+D=0
	double a, b, d;
};

struct Camera{
	double f; // focal length
	double k1, k2; // radial distortion coeffs
	double rotation[3][3];
	double translation[3];
};

class BundleOut {
public:
	int num_cameras;
	int num_points;

	Camera* camera;
	int** points;
	Position* points_3d;
	void parse_file(char* filename);
	BundleOut();
	~BundleOut();
};

#endif