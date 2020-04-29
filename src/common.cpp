#include "main.hpp"

float DegToRad(int deg)
{
    return (deg*PI/180.0);
}

float RadToDeg(float rad)
{
    return (rad/PI*180);
}

void AddAxis(coords* xyz, int n, axis a)
{
    if (a==axis::x) xyz->x += n;
    if (a==axis::y) xyz->y += n;
    if (a==axis::z) xyz->z += n;
}

coords Index_to_XYZ(int index, int width)
{
    return {(index % width), ((index / width)%width), (index / (width*width))};
}

int XYZ_to_index(coords xyz, int width)
{
    return xyz.z*width*width + xyz.y*width + xyz.x;
}

vertex Rotate(vertex v, angles a) {
    // todo: combine these into one Quat
    Quat z_q = {cos(a.psi*0.5), 0, 0, sin(a.psi*0.5)}; // unit vector at z=1
    Quat inv_z_q = {z_q.r, -z_q.i, -z_q.j, -z_q.k};

    Quat y_q = {cos(a.theta*0.5), 0, sin(a.theta*0.5), 0}; // unit vector at y=1
    Quat inv_y_q = {y_q.r, -y_q.i, -y_q.j, -y_q.k};

    Quat x_q = {cos(a.phi*0.5), sin(a.phi*0.5), 0, 0}; // unit vector at x=1
    Quat inv_x_q = {x_q.r, -x_q.i, -x_q.j, -x_q.k};

    Quat p = {0, v.x, v.y, v.z};
    p = z_q * p * inv_z_q;
    p = y_q * p * inv_y_q;
    p = x_q * p * inv_x_q;
    return {p.i, p.j, p.k};
}
