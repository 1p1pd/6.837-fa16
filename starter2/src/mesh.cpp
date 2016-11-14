#include "mesh.h"

#include "vertexrecorder.h"

using namespace std;

void Mesh::load( const char* filename )
{
	// 4.1. load() should populate bindVertices, currentVertices, and faces

	string line;
	ifstream file(filename);
	if (file.is_open()) {
		while (getline(file, line)) {
			stringstream ss(line);
			Vector3f v;
			Tuple3u f;
			string s;
			ss >> s;
			if (s == "v") {
				ss >> v[0] >> v[1] >> v[2];
				bindVertices.push_back(v);
			} else if (s == "f") {
				ss >> f[0] >> f[1] >> f[2];
				faces.push_back(f);
			}
		}
		file.close();
	}

	// make a copy of the bind vertices as the current vertices
	currentVertices = bindVertices;
}

void Mesh::draw()
{
	// 4.2 Since these meshes don't have normals
	// be sure to generate a normal per triangle.
	// Notice that since we have per-triangle normals
	// rather than the analytical normals from
	// assignment 1, the appearance is "faceted".
	VertexRecorder rec;
    Vector3f* normalvec = new Vector3f[faces.size() * 3];
    int* count = new int[faces.size() * 3];
    for (unsigned i = 0; i < faces.size(); ++i) {
        count[i] = 0;
    }
	for (unsigned i = 0; i < faces.size(); ++i) {
		Tuple3u face = faces[i];
		Vector3f point1 = currentVertices[face[0] - 1];
		Vector3f point2 = currentVertices[face[1] - 1];
		Vector3f point3 = currentVertices[face[2] - 1];
		Vector3f normal = Vector3f::cross(point2 - point1, point3 - point1);
		normal.normalize();
        for (int j = 0; j < 3; ++j) {
            int index = face[j] - 1;
            if (count[index]) {
                normalvec[index] = (normalvec[index] * count[index] + normal) / (count[index] + 1);
                count[index]++;
            } else {
                normalvec[index] = normal;
                count[index]++;
            }
        }
	}
    for (unsigned i = 0; i < faces.size(); ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = faces[i][j] - 1;
            rec.record(currentVertices[index], normalvec[index]);
        }
    }
    delete[] normalvec;
    delete[] count;
	rec.draw();
}

void Mesh::loadAttachments( const char* filename, int numJoints )
{
	// 4.3. Implement this method to load the per-vertex attachment weights
	// this method should update m_mesh.attachments
	string line;
	ifstream file(filename);
	if (file.is_open()) {
		while (getline(file, line)) {
			stringstream ss(line);
			vector<float> w;
			float temp;
			w.push_back(0);
			for (int i = 0; i < numJoints - 1; ++i) {
				ss >> temp;
				w.push_back(temp);
			}
			attachments.push_back(w);
		}
		file.close();
	}
}
