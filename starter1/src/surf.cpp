#include "surf.h"
#include "vertexrecorder.h"
using namespace std;

const float c_pi = 3.14159265358979323846f;

namespace
{
    
    // We're only implenting swept surfaces where the profile curve is
    // flat on the xy-plane.  This is a check function.
    static bool checkFlat(const Curve &profile)
    {
        for (unsigned i=0; i<profile.size(); i++)
            if (profile[i].V[2] != 0.0 ||
                profile[i].T[2] != 0.0 ||
                profile[i].N[2] != 0.0)
                return false;
    
        return true;
    }
}

// DEBUG HELPER
Surface quad() { 
	Surface ret;
	ret.VV.push_back(Vector3f(-1, -1, 0));
	ret.VV.push_back(Vector3f(+1, -1, 0));
	ret.VV.push_back(Vector3f(+1, +1, 0));
	ret.VV.push_back(Vector3f(-1, +1, 0));

	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));

	ret.VF.push_back(Tup3u(0, 1, 2));
	ret.VF.push_back(Tup3u(0, 2, 3));
	return ret;
}

Surface makeSurfRev(const Curve &profile, unsigned steps)
{
    Surface surface = Surface();

    if (!checkFlat(profile))
    {
        cerr << "surfRev profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.

//    cerr << "\t>>> makeSurfRev called (but not implemented).\n\t>>> Returning empty surface." << endl;
    float delta = 2.0 * c_pi / steps;
    Matrix3f rotate = Matrix3f(cos(delta), 0, sin(delta),
                               0, 1, 0,
                               -sin(delta), 0, cos(delta));
    Curve current = profile;
    for (unsigned i = 0; i < steps; ++i) {
        for (unsigned j = 0; j < current.size(); ++j) {
            surface.VV.push_back(current[j].V);
            surface.VN.push_back(-current[j].N);
            if (j != current.size() - 1) {
                surface.VF.push_back(Tup3u(
                        i * current.size() + j,
                        i * current.size() + j + 1,
                        ((i + 1) % steps) * current.size() + j
                ));
                surface.VF.push_back(Tup3u(
                        i * current.size() + j + 1,
                        ((i + 1) % steps) * current.size() + j + 1,
                        ((i + 1) % steps) * current.size() + j
                ));
            }

        }
        for (unsigned k = 0; k < current.size(); ++k) {
            current[k].V = rotate * current[k].V;
            current[k].N = rotate * current[k].N;
        }
    }
    return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
    Surface surface;

    if (!checkFlat(profile))
    {
        cerr << "genCyl profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.

//    cerr << "\t>>> makeGenCyl called (but not implemented).\n\t>>> Returning empty surface." <<endl;
    unsigned steps = sweep.size();
    Matrix4f M, Minverse;
    Curve current = profile;
    for (unsigned i = 0; i < steps; ++i) {
        M = Matrix4f(Vector4f(sweep[i].N, 0),
                     Vector4f(sweep[i].B, 0),
                     Vector4f(sweep[i].T, 0),
                     Vector4f(sweep[i].V, 1));
        Minverse = M.inverse();
        Minverse.transpose();
        for (unsigned k = 0; k < current.size(); ++k) {
            Vector4f tempV, tempN;
            tempV = M * Vector4f(profile[k].V, 1);
            tempN = Minverse * Vector4f(profile[k].N, 1);
            current[k].V = Vector3f(tempV[0], tempV[1], tempV[2]);
            current[k].N = Vector3f(tempN[0], tempN[1], tempN[2]);
        }
        for (unsigned j = 0; j < current.size(); ++j) {
            surface.VV.push_back(current[j].V);
            surface.VN.push_back(-current[j].N);
            if (j != current.size() - 1) {
                surface.VF.push_back(Tup3u(
                        i * current.size() + j,
                        i * current.size() + j + 1,
                        ((i + 1) % steps) * current.size() + j
                ));
                surface.VF.push_back(Tup3u(
                        i * current.size() + j + 1,
                        ((i + 1) % steps) * current.size() + j + 1,
                        ((i + 1) % steps) * current.size() + j
                ));
            }
        }
    }
    return surface;
}

void recordSurface(const Surface &surface, VertexRecorder* recorder) {
	const Vector3f WIRECOLOR(0.4f, 0.4f, 0.4f);
    for (int i=0; i<(int)surface.VF.size(); i++)
    {
		recorder->record(surface.VV[surface.VF[i][0]], surface.VN[surface.VF[i][0]], WIRECOLOR);
		recorder->record(surface.VV[surface.VF[i][1]], surface.VN[surface.VF[i][1]], WIRECOLOR);
		recorder->record(surface.VV[surface.VF[i][2]], surface.VN[surface.VF[i][2]], WIRECOLOR);
    }
}

void recordNormals(const Surface &surface, VertexRecorder* recorder, float len)
{
	const Vector3f NORMALCOLOR(0, 1, 1);
    for (int i=0; i<(int)surface.VV.size(); i++)
    {
		recorder->record_poscolor(surface.VV[i], NORMALCOLOR);
		recorder->record_poscolor(surface.VV[i] + surface.VN[i] * len, NORMALCOLOR);
    }
}

void outputObjFile(ostream &out, const Surface &surface)
{
    
    for (int i=0; i<(int)surface.VV.size(); i++)
        out << "v  "
            << surface.VV[i][0] << " "
            << surface.VV[i][1] << " "
            << surface.VV[i][2] << endl;

    for (int i=0; i<(int)surface.VN.size(); i++)
        out << "vn "
            << surface.VN[i][0] << " "
            << surface.VN[i][1] << " "
            << surface.VN[i][2] << endl;

    out << "vt  0 0 0" << endl;
    
    for (int i=0; i<(int)surface.VF.size(); i++)
    {
        out << "f  ";
        for (unsigned j=0; j<3; j++)
        {
            unsigned a = surface.VF[i][j]+1;
            out << a << "/" << "1" << "/" << a << " ";
        }
        out << endl;
    }
}
