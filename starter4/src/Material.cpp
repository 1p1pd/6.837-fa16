#include "Material.h"
Vector3f Material::shade(const Ray &ray,
    const Hit &hit,
    const Vector3f &dirToLight,
    const Vector3f &lightIntensity)
{
    // TODO implement Diffuse and Specular phong terms
    Vector3f diffuse, specular;
    float clamp = Vector3f::dot(dirToLight, hit.getNormal());
    if (clamp > 0) {
        diffuse = clamp * _diffuseColor * lightIntensity;
    } else {
        diffuse = Vector3f(0, 0, 0);
    }
    Vector3f R = ray.getDirection() - 2 * Vector3f::dot(ray.getDirection(), hit.getNormal()) * hit.getNormal();
    clamp = Vector3f::dot(dirToLight, R);
    if (clamp > 0) {
        specular = pow(clamp, _shininess) * _specularColor * lightIntensity;
    } else if (_shininess == 0) {
        specular = _specularColor * lightIntensity;
    } else {
        specular = Vector3f(0, 0, 0);
    }
    return diffuse + specular;
}
