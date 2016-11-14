#include <limits>
#include "curve.h"
#include "vertexrecorder.h"
using namespace std;

const float c_pi = 3.14159265358979323846f;

namespace
{
// Approximately equal to.  We don't want to use == because of
// precision issues with floating point.
inline bool approx(const Vector3f& lhs, const Vector3f& rhs)
{
	const float eps = 1e-8f;
	return (lhs - rhs).absSquared() < eps;
}


}


Curve evalBezier(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4 || P.size() % 3 != 1)
	{
		cerr << "evalBezier must be called with 3n+1 control points." << endl;
		exit(0);
	}

	// TODO:
	// You should implement this function so that it returns a Curve
	// (e.g., a vector< CurvePoint >).  The variable "steps" tells you
	// the number of points to generate on each piece of the spline.
	// At least, that's how the sample solution is implemented and how
	// the SWP files are written.  But you are free to interpret this
	// variable however you want, so long as you can control the
	// "resolution" of the discretized spline curve with it.

	// Make sure that this function computes all the appropriate
	// Vector3fs for each CurvePoint: V,T,N,B.
	// [NBT] should be unit and orthogonal.

	// Also note that you may assume that all Bezier curves that you
	// receive have G1 continuity.  Otherwise, the TNB will not be
	// be defined at points where this does not hold.

//	cerr << "\t>>> evalBezier has been called with the following input:" << endl;
//
//	cerr << "\t>>> Control points (type vector< Vector3f >): " << endl;
//	for (int i = 0; i < (int)P.size(); ++i)
//	{
//		cerr << "\t>>> " << P[i] << endl;
//	}
//
//	cerr << "\t>>> Steps (type steps): " << steps << endl;
//	cerr << "\t>>> Returning empty curve." << endl;

	float delta = (float)1.0 / steps;
	Vector4f x, y, z;
	Vector3f V, T, N, B;
	Curve result = Curve();
	Matrix4f bernstein = Matrix4f(1, -3, 3, -1,
		 						  0, 3, -6, 3,
								  0, 0, 3, -3,
								  0, 0, 0, 1);
	for (int i = 3; i < (int)P.size(); i += 3) {
		x = Vector4f(P[i - 3][0], P[i - 2][0], P[i - 1][0], P[i][0]);
		y = Vector4f(P[i - 3][1], P[i - 2][1], P[i - 1][1], P[i][1]);
		z = Vector4f(P[i - 3][2], P[i - 2][2], P[i - 1][2], P[i][2]);
		V = P[i - 3];
		T = P[i - 2] - P[i - 3];
		B = Vector3f(0, 0, 1);
		N = Vector3f::cross(B, T);
		T.normalize();
		N.normalize();
		B.normalize();
		struct CurvePoint current;
		for (unsigned j = 0; j <= steps; j++) {
			float t = delta * j;
			Vector4f basis = bernstein * Vector4f(1, t, t * t, t * t * t);
			Vector4f basisder = bernstein * Vector4f(0, 1, 2 * t, 3 * t * t);
			V = Vector3f(Vector4f::dot(basis, x), Vector4f::dot(basis, y), Vector4f::dot(basis, z));
			T = Vector3f(Vector4f::dot(basisder, x), Vector4f::dot(basisder, y), Vector4f::dot(basisder, z));
			N = Vector3f::cross(B, T);
			B = Vector3f::cross(T, N);
			T.normalize();
			N.normalize();
			B.normalize();
			current = {V, T, N, B};
			result.push_back(current);
		}
	}
	// Right now this will just return this empty curve.
	return result;
}

bool isSame(float a, float b) {
    return fabsf(a - b) <= 0.000001;
}

Curve evalBspline(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4)
	{
		cerr << "evalBspline must be called with 4 or more control points." << endl;
		exit(0);
	}

	// TODO:
	// It is suggested that you implement this function by changing
	// basis from B-spline to Bezier.  That way, you can just call
	// your evalBezier function.

//	cerr << "\t>>> evalBSpline has been called with the following input:" << endl;
//
//	cerr << "\t>>> Control points (type vector< Vector3f >): " << endl;
//	for (int i = 0; i < (int)P.size(); ++i)
//	{
//		cerr << "\t>>> " << P[i] << endl;
//	}
//
//	cerr << "\t>>> Steps (type steps): " << steps << endl;
//	cerr << "\t>>> Returning empty curve." << endl;

	float delta = (float)1.0 / steps;
	Vector4f x, y, z;
	Vector3f V, T, N, B;
	Curve result = Curve();
	Matrix4f bspline = Matrix4f(1, -3, 3, -1,
								4, 0, -6, 3,
								1, 3, 3, -3,
								0, 0, 0, 1);
	bspline /= 6.0;
B = Vector3f(0, 0, 1);
	for (int i = 3; i < (int)P.size(); i++) {
		x = Vector4f(P[i - 3][0], P[i - 2][0], P[i - 1][0], P[i][0]);
		y = Vector4f(P[i - 3][1], P[i - 2][1], P[i - 1][1], P[i][1]);
		z = Vector4f(P[i - 3][2], P[i - 2][2], P[i - 1][2], P[i][2]);
		V = P[i - 3];
		T = P[i - 2] - P[i - 3];
		
		cout << B[0] << B[1] << B[2] << endl;
		N = Vector3f::cross(B, T);
		T.normalize();
		N.normalize();
		B.normalize();
		struct CurvePoint current;
		for (unsigned j = 0; j <= steps; j++) {
			float t = delta * j;
			Vector4f basis = bspline * Vector4f(1, t, t * t, t * t * t);
			Vector4f basisder = bspline * Vector4f(0, 1, 2 * t, 3 * t * t);
			V = Vector3f(Vector4f::dot(basis, x), Vector4f::dot(basis, y), Vector4f::dot(basis, z));
			T = Vector3f(Vector4f::dot(basisder, x), Vector4f::dot(basisder, y), Vector4f::dot(basisder, z));
			N = Vector3f::cross(B, T);
			B = Vector3f::cross(T, N);
			T.normalize();
			N.normalize();
			B.normalize();
			current = {V, T, N, B};
			result.push_back(current);
		}
	}
    bool status = true;
    for (int i = 0; i < 3; ++i) {
        if (!isSame(result.front().V[i], result.back().V[i])) {
            status = false;
        }
    }
    for (int i = 0; i < 3; ++i) {
        if (!isSame(result.front().T[i], result.back().T[i])) {
            status = false;
        }
    }
    bool tempstatus = true;
    for (int i = 0; i < 3; ++i) {
        if (!isSame(result.front().N[i], result.back().N[i])) {
            tempstatus = false;
            break;
        }
    }
    status = status && (!tempstatus);
    if (status) {
        float angle = acos(Vector3f::dot(result.front().N, result.back().N));
        for (unsigned i = 0; i < result.size(); ++i) {
            float x = result[i].T[0];
            float y = result[i].T[1];
            float z = result[i].T[2];
            Matrix3f rotate = Matrix3f(cos(angle)+x*x*(1-cos(angle)), x*y*(1-cos(angle))-z*sin(angle), x*z*(1-cos(angle))+y*sin(angle),
                                       y*x*(1-cos(angle))+z*sin(angle), cos(angle)+y*y*(1-cos(angle)), y*z*(1-cos(angle))-x*sin(angle),
                                       z*x*(1-cos(angle))-y*sin(angle), z*y*(1-cos(angle))+x*sin(angle), cos(angle)+z*z*(1-cos(angle)));
            //result[i].B = Vector3f(0, 0, 1);
            //result[i].N = Vector3f::cross(result[i].B, result[i].T);
            //result[i].N.normalize();
        }
    }
	// Return an empty curve right now.
	return result;
}

Curve evalCircle(float radius, unsigned steps)
{
	// This is a sample function on how to properly initialize a Curve
	// (which is a vector< CurvePoint >).

	// Preallocate a curve with steps+1 CurvePoints
	Curve R(steps + 1);

	// Fill it in counterclockwise
	for (unsigned i = 0; i <= steps; ++i)
	{
		// step from 0 to 2pi
		float t = 2.0f * c_pi * float(i) / steps;

		// Initialize position
		// We're pivoting counterclockwise around the y-axis
		R[i].V = radius * Vector3f(cos(t), sin(t), 0);

		// Tangent vector is first derivative
		R[i].T = Vector3f(-sin(t), cos(t), 0);

		// Normal vector is second derivative
		R[i].N = Vector3f(-cos(t), -sin(t), 0);

		// Finally, binormal is facing up.
		R[i].B = Vector3f(0, 0, 1);
	}

	return R;
}

void recordCurve(const Curve& curve, VertexRecorder* recorder)
{
	const Vector3f WHITE(1, 1, 1);
	for (int i = 0; i < (int)curve.size() - 1; ++i)
	{
		recorder->record_poscolor(curve[i].V, WHITE);
		recorder->record_poscolor(curve[i + 1].V, WHITE);
	}
}
void recordCurveFrames(const Curve& curve, VertexRecorder* recorder, float framesize)
{
	Matrix4f T;
	const Vector3f RED(1, 0, 0);
	const Vector3f GREEN(0, 1, 0);
	const Vector3f BLUE(0, 0, 1);

	const Vector4f ORGN(0, 0, 0, 1);
	const Vector4f AXISX(framesize, 0, 0, 1);
	const Vector4f AXISY(0, framesize, 0, 1);
	const Vector4f AXISZ(0, 0, framesize, 1);

	for (int i = 0; i < (int)curve.size(); ++i)
	{
		T.setCol(0, Vector4f(curve[i].N, 0));
		T.setCol(1, Vector4f(curve[i].B, 0));
		T.setCol(2, Vector4f(curve[i].T, 0));
		T.setCol(3, Vector4f(curve[i].V, 1));

		// Transform orthogonal frames into model space
		Vector4f MORGN  = T * ORGN;
		Vector4f MAXISX = T * AXISX;
		Vector4f MAXISY = T * AXISY;
		Vector4f MAXISZ = T * AXISZ;

		// Record in model space
		recorder->record_poscolor(MORGN.xyz(), RED);
		recorder->record_poscolor(MAXISX.xyz(), RED);

		recorder->record_poscolor(MORGN.xyz(), GREEN);
		recorder->record_poscolor(MAXISY.xyz(), GREEN);

		recorder->record_poscolor(MORGN.xyz(), BLUE);
		recorder->record_poscolor(MAXISZ.xyz(), BLUE);
	}
}
