#include "Object3D.h"
#include "VecUtils.h"

bool Sphere::intersect(const Ray &r, float tmin, Hit &h) const
{
    // BEGIN STARTER

    // We provide sphere intersection code for you.
    // You should model other intersection implementations after this one.

    // Locate intersection point ( 2 pts )
    const Vector3f &rayOrigin = r.getOrigin(); //Ray origin in the world coordinate
    const Vector3f &dir = r.getDirection();

    Vector3f origin = rayOrigin - _center;      //Ray origin in the sphere coordinate

    float a = dir.absSquared();
    float b = 2 * Vector3f::dot(dir, origin);
    float c = origin.absSquared() - _radius * _radius;

    // no intersection
    if (b * b - 4 * a * c < 0) {
        return false;
    }

    float d = sqrt(b * b - 4 * a * c);

    float tplus = (-b + d) / (2.0f*a);
    float tminus = (-b - d) / (2.0f*a);

    // the two intersections are at the camera back
    if ((tplus < tmin) && (tminus < tmin)) {
        return false;
    }

    float t = 10000;
    // the two intersections are at the camera front
    if (tminus > tmin) {
        t = tminus;
    }

    // one intersection at the front. one at the back 
    if ((tplus > tmin) && (tminus < tmin)) {
        t = tplus;
    }

    if (t < h.getT()) {
        Vector3f normal = r.pointAtParameter(t) - _center;
        normal = normal.normalized();
        h.set(t, this->material, normal);
        return true;
    }
    // END STARTER
    return false;
}

// Add object to group
void Group::addObject(Object3D *obj) {
    m_members.push_back(obj);
}

// Return number of objects in group
int Group::getGroupSize() const {
    return (int)m_members.size();
}

bool Group::intersect(const Ray &r, float tmin, Hit &h) const
{
    // BEGIN STARTER
    // we implemented this for you
    bool hit = false;
    for (Object3D* o : m_members) {
        if (o->intersect(r, tmin, h)) {
            hit = true;
        }
    }
    return hit;
    // END STARTER
}


Plane::Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
    // TODO implement Plane constructor
    this->_dist = -d;
    this->_normal = normal;
}
bool Plane::intersect(const Ray &r, float tmin, Hit &h) const
{
    // TODO implement
    float t = -(_dist + Vector3f::dot(_normal, r.getOrigin())) / Vector3f::dot(_normal, r.getDirection());
    if (t > tmin && t < h.getT()) {
        h.set(t, this->material, _normal.normalized());
        return true;
    }
    return false;
}
bool Triangle::intersect(const Ray &r, float tmin, Hit &h) const 
{
    // TODO implement
    Matrix3f A = Matrix3f(_v[0] - _v[1], _v[0] - _v[2], r.getDirection());
    Vector3f b = _v[0] - r.getOrigin();
    Vector3f result = A.inverse() * b;
    if (result[2] > tmin && result[2] < h.getT() && result[0] + result[1] <= 1 &&
            result[0] >= 0 && result[1] >= 0) {
        h.set(result[2], this->material, ((1 - result[0] - result[1]) * _normals[0]
                                        + result[0] * _normals[1]
                                        + result[1] * _normals[2]).normalized());
        return true;
    }
    return false;
}


Transform::Transform(const Matrix4f &m,
    Object3D *obj) : _object(obj) {
    // TODO implement Transform constructor
    this->_transform = m;
}
bool Transform::intersect(const Ray &r, float tmin, Hit &h) const
{
    // TODO implement
    Ray objworld_r = Ray(VecUtils::transformPoint(_transform.inverse(), r.getOrigin()),
                        VecUtils::transformDirection(_transform.inverse(), r.getDirection()));
    bool result = _object->intersect(objworld_r, tmin, h);
    if (result) {
        h.set(h.getT(), h.getMaterial(), (_transform.getSubmatrix3x3(0, 0).inverse().transposed() * h.getNormal()).normalized());
        return true;
    }
    return false;
}