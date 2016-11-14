#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "Ray.h"
#include "Image.h"
#include "Vector3f.h"

#include <string>

#include <stdlib.h>
#include <math.h>
#include <cstdio>

#define BRDF_SAMPLING_RES_THETA_H       90
#define BRDF_SAMPLING_RES_THETA_D       90
#define BRDF_SAMPLING_RES_PHI_D         360

#define RED_SCALE (1.0/1500.0)
#define GREEN_SCALE (1.15/1500.0)
#define BLUE_SCALE (1.66/1500.0)
#define M_PI	3.1415926535897932384626433832795

class Material
{
  public:
    Material(const Vector3f &diffuseColor, 
             const Vector3f &specularColor = Vector3f::ZERO, 
             float shininess = 0) :
        _diffuseColor(diffuseColor),
        _specularColor(specularColor),
        _shininess(shininess)
    {
        FILE *f = fopen("/home/james/Desktop/BRDF/BRDFs/orange-paint.binary", "rb");
        int dims[3];
        fread(dims, sizeof(int), 3, f);
        int n = dims[0] * dims[1] * dims[2];
        brdf = (double*) malloc(sizeof(double) * 3 * n);
        fread(brdf, sizeof(double), 3 * n, f);
        fclose(f);
    }

    const Vector3f & getDiffuseColor() const {
        return _diffuseColor;
    }

    const Vector3f & getSpecularColor() const {
        return _specularColor;
    }

    Vector3f shade(const Ray &ray,
        const Hit &hit,
        const Vector3f &dirToLight,
        const Vector3f &lightIntensity);

protected:

    Vector3f _diffuseColor;
    Vector3f _specularColor;
    float   _shininess;
    double* brdf;
};

#endif // MATERIAL_H
