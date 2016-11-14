#include "clothsystem.h"
#include "camera.h"
#include "vertexrecorder.h"

 // your system should at least contain 8x8 particles.
const int W = 8;
const int H = 8;

ClothSystem::ClothSystem()
{
    // TODO 5. Initialize m_vVecState with cloth particles. 
    // You can again use rand_uniform(lo, hi) to make things a bit more interesting
    std::vector<Vector3f> s;
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            s.push_back(Vector3f(j * 0.5, 0, -i * 0.5 + rand_uniform(-0.05, 0.05)));
            s.push_back(Vector3f(0, 0, 0));
        }
    }
    this->setState(s);
}

Vector3f ClothForce(int iy, int ix, int jy, int jx, std::vector<Vector3f> pos, float k, float l) {
    if (jx < 0 || jx >= W || ix < 0 || ix >= W) {
        return Vector3f(0, 0, 0);
    }
    if (iy < 0 || iy >= H || jy < 0 || jy >= H) {
        return Vector3f(0, 0, 0);
    }
    Vector3f posi, posj, d;
    posi = pos[iy * W + ix];
    posj = pos[jy * W + jx];
    d = posi - posj;
    return -k * (d.abs() - l) * d.normalized();
}

void ClothSystem::addWind() {
    this->isWind = !this->isWind;
}

std::vector<Vector3f> ClothSystem::evalF(std::vector<Vector3f> state)
{
    // TODO 5. implement evalF
    std::vector<Vector3f> f, pos, vel;
    float m, g, kst, ksh, kf, d, l;
    m = 0.01;
    g = 9.8;
    kst = 1;
    ksh = 0.5;
    kf = 10;
    d = 0.01;
    l = 0.5;
    for (unsigned i = 0; i < state.size(); ++i) {
        if (i & 1) {
            vel.push_back(state[i]);
        }
        else {
            pos.push_back(state[i]);
        }
    }
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            if ((i == 0 && j == 0) || (i == 0 && j == W - 1)) {
                f.push_back(Vector3f(0, 0, 0));
                f.push_back(Vector3f(0, 0, 0));
                continue;
            }
            f.push_back(vel[i * W + j]);
            Vector3f gravity, drag, spring, force;
            gravity = m * Vector3f(0, -g, 0);
            drag = -d * vel[i * W + j];
            spring = ClothForce(i, j, i + 1, j, pos, kst, l)
                     + ClothForce(i, j, i - 1, j, pos, kst, l)
                     + ClothForce(i, j, i, j + 1, pos, kst, l)
                     + ClothForce(i, j, i, j - 1, pos, kst, l);
            spring += ClothForce(i, j, i + 2, j, pos, ksh, 2 * l)
                     + ClothForce(i, j, i - 2, j, pos, ksh, 2 * l)
                     + ClothForce(i, j, i, j + 2, pos, ksh, 2 * l)
                     + ClothForce(i, j, i, j - 2, pos, ksh, 2 * l);
            spring += ClothForce(i, j, i + 1, j + 1, pos, kf, sqrt(2) * l)
                      + ClothForce(i, j, i + 1, j - 1, pos, kf, sqrt(2) * l)
                      + ClothForce(i, j, i - 1, j + 1, pos, kf, sqrt(2) * l)
                      + ClothForce(i, j, i - 1, j - 1, pos, kf, sqrt(2) * l);
            force = gravity + drag + spring;
            if (this->isWind) {
                force += Vector3f(rand_uniform(-0.5, 0.5), rand_uniform(-0.5, 0.5), rand_uniform(-0.5, 0.5));
            }
            f.push_back(force / m);
        }
    }
    return f;
}


void ClothSystem::draw(GLProgram& gl)
{
    //TODO 5: render the system
    //         - ie draw the particles as little spheres
    //         - or draw the springs as little lines or cylinders
    //         - or draw wireframe mesh

//    const Vector3f CLOTH_COLOR(0.9f, 0.9f, 0.9f);
//    gl.updateMaterial(CLOTH_COLOR);
//    gl.updateModelMatrix(Matrix4f::identity());

    // EXAMPLE for how to render cloth particles.
    //  - you should replace this code.
//    for (unsigned i = 0; i < this->getState().size(); i += 2) {
//        gl.updateMaterial(CLOTH_COLOR);
//        gl.updateModelMatrix(Matrix4f::translation(this->getState()[i]));
//        drawSphere(0.04f, 8, 8);
//    }

    // EXAMPLE: This shows you how to render lines to debug the spring system.
    //
    //          You should replace this code.
    //
    //          Since lines don't have a clearly defined normal, we can't use
    //          a regular lighting model.
    //          GLprogram has a "color only" mode, where illumination
    //          is disabled, and you specify color directly as vertex attribute.
    //          Note: enableLighting/disableLighting invalidates uniforms,
    //          so you'll have to update the transformation/material parameters
    //          after a mode change.
//    gl.disableLighting();
//    gl.updateModelMatrix(Matrix4f::identity()); // update uniforms after mode change
//    VertexRecorder rec;
//    for (int i = 0; i < H; ++i) {
//        for (int j = 0; j < W - 1; ++j) {
//            rec.record(this->getState()[(i * W + j) * 2], CLOTH_COLOR);
//            rec.record(this->getState()[(i * W + j + 1) * 2], CLOTH_COLOR);
//        }
//    }
//    for (int i = 0; i < H - 1; ++i) {
//        for (int j = 0; j < W; ++j) {
//            rec.record(this->getState()[(i * W + j) * 2], CLOTH_COLOR);
//            rec.record(this->getState()[((i + 1) * W + j) * 2], CLOTH_COLOR);
//        }
//    }
//    glLineWidth(3.0f);
//    rec.draw(GL_LINES);
//
//    gl.enableLighting(); // reset to default lighting model
    // EXAMPLE END
    const Vector3f CLOTH_COLOR(0.9f, 0.9f, 0.9f);
    gl.updateMaterial(CLOTH_COLOR);
    gl.updateModelMatrix(Matrix4f::identity());
    std::vector<Vector3f> pos;
    VertexRecorder recod;
    std::vector<Tuple3i> faces;
    Vector3f* normalVec = new Vector3f[W * H];
    unsigned* count = new unsigned[W * H];
    for (unsigned i = 0; i < faces.size(); ++i) {
        count[i] = 0;
    }
    for (unsigned i = 0; i < this->getState().size(); i += 2) {
        pos.push_back(this->getState()[i]);
    }
    for (int i = 0; i < H - 1; ++i) {
        for (int j = 0; j < W - 1; ++j) {
            faces.push_back(Tuple3i(i * W + j, i * W + j + 1, (i + 1) * W + j));
            faces.push_back(Tuple3i((i + 1) * W + j, i * W + j + 1, (i + 1) * W + j + 1));
        }
    }
    for (unsigned i = 0; i < faces.size(); ++i) {
        Vector3f p1, p2, p3, n;
        Tuple3i face = faces[i];
        p1 = pos[face[0]];
        p2 = pos[face[1]];
        p3 = pos[face[2]];
        n = -Vector3f::cross(p2 - p1, p3 - p1).normalized();
        for (int j = 0; j < 3; ++j) {
            int index = face[j];
            if (count[index]) {
                normalVec[index] = (normalVec[index] * count[index] + n) / (count[index] + 1);
                count[index]++;
            } else {
                normalVec[index] = n;
                count[index]++;
            }
        }
    }
    for (unsigned i = 0; i < faces.size(); ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = faces[i][j];
            recod.record(pos[index], normalVec[index]);
        }
    }
    recod.draw();
    delete[] count;
    delete[] normalVec;
}

