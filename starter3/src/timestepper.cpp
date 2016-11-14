#include "timestepper.h"

#include <cstdio>

void ForwardEuler::takeStep(ParticleSystem* particleSystem, float stepSize)
{
   //TODO: See handout 3.1
    std::vector<Vector3f> current, d, next;
    current = particleSystem->getState();
    d = particleSystem->evalF(current);
    for (unsigned i = 0; i < current.size(); ++i) {
        next.push_back(current[i] + d[i] * stepSize);
    }
    particleSystem->setState(next);
}

void Trapezoidal::takeStep(ParticleSystem* particleSystem, float stepSize)
{
   //TODO: See handout 3.1
    std::vector<Vector3f> current, d1, d2, next;
    current = particleSystem->getState();
    d1 = particleSystem->evalF(current);
    for (unsigned i = 0; i < current.size(); ++i) {
        next.push_back(current[i] + d1[i] * stepSize);
    }
    d2 = particleSystem->evalF(next);
    std::vector<Vector3f> result;
    for (unsigned j = 0; j < current.size(); ++j) {
        result.push_back(current[j] + 0.5 * (d1[j] + d2[j]) * stepSize);
    }
    particleSystem->setState(result);
}


void RK4::takeStep(ParticleSystem* particleSystem, float stepSize)
{
   //TODO: See handout 4.4
    std::vector<Vector3f> current, k1, k2, k3, k4, next;
    std::vector<Vector3f> k1s, k2s, k3s;
    current = particleSystem->getState();
    k1 = particleSystem->evalF(current);
    for (unsigned i = 0; i < current.size(); ++i) {
        k1s.push_back(current[i] + k1[i] * stepSize / 2);
    }
    k2 = particleSystem->evalF(k1s);
    for (unsigned i = 0; i < current.size(); ++i) {
        k2s.push_back(current[i] + k2[i] * stepSize / 2);
    }
    k3 = particleSystem->evalF(k2s);
    for (unsigned i = 0; i < current.size(); ++i) {
        k3s.push_back(current[i] + k3[i] * stepSize);
    }
    k4 = particleSystem->evalF(k3s);
    for (unsigned i = 0; i < current.size(); ++i) {
        next.push_back(current[i] + (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) * stepSize / 6);
    }
    particleSystem->setState(next);
}

