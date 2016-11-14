#ifndef CLOTHSYSTEM_H
#define CLOTHSYSTEM_H

#include <vector>

#include "particlesystem.h"
#include "tuple.h"

typedef tuple<int, 3> Tuple3i;

class ClothSystem : public ParticleSystem
{
    ///ADD MORE FUNCTION AND FIELDS HERE
private:
    bool isWind = false;
public:
    ClothSystem();

    // evalF is called by the integrator at least once per time step
    std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;

    // draw is called once per frame
    void draw(GLProgram& ctx);
    void addWind();
    // inherits
    // std::vector<Vector3f> m_vVecState;
};


#endif
