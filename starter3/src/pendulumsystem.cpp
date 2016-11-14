#include "pendulumsystem.h"

#include <cassert>
#include "camera.h"
#include "vertexrecorder.h"

// TODO adjust to number of particles.
const int NUM_PARTICLES = 5;

PendulumSystem::PendulumSystem()
{

    // TODO 4.2 Add particles for simple pendulum
    // TODO 4.3 Extend to multiple particles

    // To add a bit of randomness, use e.g.
    // float f = rand_uniform(-0.5f, 0.5f);
    // in your initial conditions.
    std::vector<Vector3f> s;
    s.push_back(Vector3f(-2, 2, 0));
    s.push_back(Vector3f(0, 0, 0));
    for (int i = 1; i < NUM_PARTICLES; ++i) {
        s.push_back(Vector3f(rand_uniform(-2.5f, -1.5f), rand_uniform(1.5f, 2.5f), 0));
        s.push_back(Vector3f(0, 0, 0));
    }
    this->setState(s);
}

Vector3f SpringForce(int i, int j, std::vector<Vector3f> pos, float k, float l) {
    if (i < 0 || i >= NUM_PARTICLES || j < 0 || j >= NUM_PARTICLES) {
        return Vector3f(0, 0, 0);
    }
    Vector3f posi, posj, d;
    posi = pos[i];
    posj = pos[j];
    d = posi - posj;
    return -k * (d.abs() - l) * d.normalized();
}

std::vector<Vector3f> PendulumSystem::evalF(std::vector<Vector3f> state)
{
    std::vector<Vector3f> f, pos, vel;
    // TODO 4.1: implement evalF
    float m, g, k, d, l;
    m = 0.05;
    g = 9.8;
    k = 1.5;
    d = 0.02;
    l = 0.3;
    for (unsigned i = 0; i < state.size(); ++i) {
        if (i & 1) {
            vel.push_back(state[i]);
        }
        else {
            pos.push_back(state[i]);
        }
    }
    f.push_back(Vector3f(0, 0, 0));
    f.push_back(Vector3f(0, 0, 0));
    for (int i = 1; i < NUM_PARTICLES; ++i) {
        f.push_back(vel[i]);
        Vector3f gravity, drag, spring, force;
        gravity = m * Vector3f(0, -g, 0);
        drag = -d * vel[i];
        spring = SpringForce(i, i + 1, pos, k, l) + SpringForce(i, i - 1, pos, k, l);
//        spring += SpringForce(i, i + 2, pos, k, l) + SpringForce(i, i - 2, pos, k, l);
        force = gravity + drag + spring;
        f.push_back(force / m);
    }
    return f;
}

// render the system (ie draw the particles)
void PendulumSystem::draw(GLProgram& gl)
{
    for (unsigned i = 0; i < this->getState().size(); i += 2) {
        const Vector3f PENDULUM_COLOR(0.73f, 0.0f, 0.83f);
        gl.updateMaterial(PENDULUM_COLOR);
        gl.updateModelMatrix(Matrix4f::translation(this->getState()[i]));
        drawSphere(0.075f, 10, 10);
    }
}
