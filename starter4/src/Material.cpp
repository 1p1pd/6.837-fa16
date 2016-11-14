#include "Material.h"

// cross product of two vectors
void cross_product (double* v1, double* v2, double* out)
{
    out[0] = v1[1]*v2[2] - v1[2]*v2[1];
    out[1] = v1[2]*v2[0] - v1[0]*v2[2];
    out[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

// normalize vector
void normalize(double* v)
{
    // normalize
    double len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    v[0] = v[0] / len;
    v[1] = v[1] / len;
    v[2] = v[2] / len;
}

// rotate vector along one axis
void rotate_vector(double* vector, double* axis, double angle, double* out)
{
    double temp;
    double cross[3];
    double cos_ang = cos(angle);
    double sin_ang = sin(angle);

    out[0] = vector[0] * cos_ang;
    out[1] = vector[1] * cos_ang;
    out[2] = vector[2] * cos_ang;

    temp = axis[0]*vector[0]+axis[1]*vector[1]+axis[2]*vector[2];
    temp = temp*(1.0-cos_ang);

    out[0] += axis[0] * temp;
    out[1] += axis[1] * temp;
    out[2] += axis[2] * temp;

    cross_product (axis,vector,cross);

    out[0] += cross[0] * sin_ang;
    out[1] += cross[1] * sin_ang;
    out[2] += cross[2] * sin_ang;
}


// convert standard coordinates to half vector/difference vector coordinates
void std_coords_to_half_diff_coords(double theta_in, double fi_in, double theta_out, double fi_out,
                                    double& theta_half,double& fi_half,double& theta_diff,double& fi_diff )
{

    // compute in vector
    double in_vec_z = cos(theta_in);
    double proj_in_vec = sin(theta_in);
    double in_vec_x = proj_in_vec*cos(fi_in);
    double in_vec_y = proj_in_vec*sin(fi_in);
    double in[3]= {in_vec_x,in_vec_y,in_vec_z};
    normalize(in);


    // compute out vector
    double out_vec_z = cos(theta_out);
    double proj_out_vec = sin(theta_out);
    double out_vec_x = proj_out_vec*cos(fi_out);
    double out_vec_y = proj_out_vec*sin(fi_out);
    double out[3]= {out_vec_x,out_vec_y,out_vec_z};
    normalize(out);


    // compute halfway vector
    double half_x = (in_vec_x + out_vec_x)/2.0f;
    double half_y = (in_vec_y + out_vec_y)/2.0f;
    double half_z = (in_vec_z + out_vec_z)/2.0f;
    double half[3] = {half_x,half_y,half_z};
    normalize(half);

    // compute  theta_half, fi_half
    theta_half = acos(half[2]);
    fi_half = atan2(half[1], half[0]);


    double bi_normal[3] = {0.0, 1.0, 0.0};
    double normal[3] = { 0.0, 0.0, 1.0 };
    double temp[3];
    double diff[3];

    // compute diff vector
    rotate_vector(in, normal , -fi_half, temp);
    rotate_vector(temp, bi_normal, -theta_half, diff);

    // compute  theta_diff, fi_diff
    theta_diff = acos(diff[2]);
    fi_diff = atan2(diff[1], diff[0]);

}


// Lookup theta_half index
// This is a non-linear mapping!
// In:  [0 .. pi/2]
// Out: [0 .. 89]
inline int theta_half_index(double theta_half)
{
    if (theta_half <= 0.0)
        return 0;
    double theta_half_deg = ((theta_half / (M_PI/2.0))*BRDF_SAMPLING_RES_THETA_H);
    double temp = theta_half_deg*BRDF_SAMPLING_RES_THETA_H;
    temp = sqrt(temp);
    int ret_val = (int)temp;
    if (ret_val < 0) ret_val = 0;
    if (ret_val >= BRDF_SAMPLING_RES_THETA_H)
        ret_val = BRDF_SAMPLING_RES_THETA_H-1;
    return ret_val;
}


// Lookup theta_diff index
// In:  [0 .. pi/2]
// Out: [0 .. 89]
inline int theta_diff_index(double theta_diff)
{
    int tmp = int(theta_diff / (M_PI * 0.5) * BRDF_SAMPLING_RES_THETA_D);
    if (tmp < 0)
        return 0;
    else if (tmp < BRDF_SAMPLING_RES_THETA_D - 1)
        return tmp;
    else
        return BRDF_SAMPLING_RES_THETA_D - 1;
}


// Lookup phi_diff index
inline int phi_diff_index(double phi_diff)
{
    // Because of reciprocity, the BRDF is unchanged under
    // phi_diff -> phi_diff + M_PI
    if (phi_diff < 0.0)
        phi_diff += M_PI;

    // In: phi_diff in [0 .. pi]
    // Out: tmp in [0 .. 179]
    int tmp = int(phi_diff / M_PI * BRDF_SAMPLING_RES_PHI_D / 2);
    if (tmp < 0)
        return 0;
    else if (tmp < BRDF_SAMPLING_RES_PHI_D / 2 - 1)
        return tmp;
    else
        return BRDF_SAMPLING_RES_PHI_D / 2 - 1;
}


// Given a pair of incoming/outgoing angles, look up the BRDF.
void lookup_brdf_val(double* brdf, double theta_in, double fi_in,
                     double theta_out, double fi_out,
                     double& red_val,double& green_val,double& blue_val)
{
    // Convert to halfangle / difference angle coordinates
    double theta_half, fi_half, theta_diff, fi_diff;

    std_coords_to_half_diff_coords(theta_in, fi_in, theta_out, fi_out,
                                   theta_half, fi_half, theta_diff, fi_diff);


    // Find index.
    // Note that phi_half is ignored, since isotropic BRDFs are assumed
    int ind = phi_diff_index(fi_diff) +
              theta_diff_index(theta_diff) * BRDF_SAMPLING_RES_PHI_D / 2 +
              theta_half_index(theta_half) * BRDF_SAMPLING_RES_PHI_D / 2 *
              BRDF_SAMPLING_RES_THETA_D;

    red_val = brdf[ind] * RED_SCALE;
    green_val = brdf[ind + BRDF_SAMPLING_RES_THETA_H*BRDF_SAMPLING_RES_THETA_D*BRDF_SAMPLING_RES_PHI_D/2] * GREEN_SCALE;
    blue_val = brdf[ind + BRDF_SAMPLING_RES_THETA_H*BRDF_SAMPLING_RES_THETA_D*BRDF_SAMPLING_RES_PHI_D] * BLUE_SCALE;


    if (red_val < 0.0 || green_val < 0.0 || blue_val < 0.0)
        fprintf(stderr, "Below horizon.\n");

}



// Read BRDF data
bool read_brdf(const char *filename, double* &brdf)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return false;

    int dims[3];
    fread(dims, sizeof(int), 3, f);
    int n = dims[0] * dims[1] * dims[2];
    if (n != BRDF_SAMPLING_RES_THETA_H *
             BRDF_SAMPLING_RES_THETA_D *
             BRDF_SAMPLING_RES_PHI_D / 2)
    {
        fprintf(stderr, "Dimensions don't match\n");
        fclose(f);
        return false;
    }

    brdf = (double*) malloc (sizeof(double)*3*n);
    fread(brdf, sizeof(double), 3*n, f);

    fclose(f);
    return true;
}

Vector3f Material::shade(const Ray &ray,
    const Hit &hit,
    const Vector3f &dirToLight,
    const Vector3f &lightIntensity)
//{
//    // TODO implement Diffuse and Specular phong terms
//    Vector3f diffuse, specular;
//    float clamp = Vector3f::dot(dirToLight, hit.getNormal());
//    if (clamp > 0) {
//        diffuse = clamp * _diffuseColor * lightIntensity;
//    } else {
//        diffuse = Vector3f(0, 0, 0);
//    }
//    Vector3f R = ray.getDirection() - 2 * Vector3f::dot(ray.getDirection(), hit.getNormal()) * hit.getNormal();
//    clamp = Vector3f::dot(dirToLight, R);
//    if (clamp > 0) {
//        specular = pow(clamp, _shininess) * _specularColor * lightIntensity;
//    } else if (_shininess == 0) {
//        specular = _specularColor * lightIntensity;
//    } else {
//        specular = Vector3f(0, 0, 0);
//    }
//    return diffuse + specular;
//}

{
    double theta_in, theta_out, phi_in, phi_out;
    float clamp = Vector3f::dot(dirToLight, hit.getNormal());
    theta_in = acos(clamp);
    clamp = Vector3f::dot(-ray.getDirection().normalized(), hit.getNormal());
    theta_out = acos(clamp);
    Vector3f t = Vector3f::cross(hit.getNormal(), Vector3f(0, 1, 0)).normalized();
    printf("%f\t%f\t%f\n", t[0], t[1], t[2]);
    if (t == Vector3f(0, 0, 0)) {
        t = Vector3f::cross(hit.getNormal(), Vector3f(0, 0, 1)).normalized();
    }
    Vector3f rayout = -ray.getDirection().normalized() - Vector3f::dot(-ray.getDirection().normalized(), hit.getNormal()) * hit.getNormal();
    clamp = Vector3f::dot(rayout.normalized(), t.normalized());
    phi_out = acos(clamp);
    printf("%f\t", clamp);
    Vector3f rayin = dirToLight - Vector3f::dot(dirToLight, hit.getNormal()) * hit.getNormal();
    clamp = Vector3f::dot(rayin.normalized(), t.normalized());
    phi_in = acos(clamp);
    printf("%f\n", clamp);
    double red, green, blue;
    printf("%f\t%f\t%f\t%f\n", theta_in, phi_in, theta_out, phi_out);
    lookup_brdf_val(brdf, theta_in, phi_in, theta_out, phi_out, red, green, blue);
    return Vector3f(red, green, blue) * lightIntensity;
}