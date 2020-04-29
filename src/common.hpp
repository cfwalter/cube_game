#ifndef COMMON_HPP
#define COMMON_HPP
#include "quat.hpp"

struct vertex  // rendering 3D graphics
{
    double x;
    double y;
    double z;
};

struct coords // track point on 3D grid
{
    int x;
    int y;
    int z;
};

struct angles
{
  double phi;
  double theta;
  double psi;
};

enum axis
{
    // TODO: how to use these only when namespaced??
    x=0,
    y,
    z,
    none,
};

enum direction
{
    up=0,
    right,
    down,
    left,
    null,
};

float DegToRad(int deg);

float RadToDeg(float rad);

void AddAxis(coords* xyz, int n, axis a);

coords Index_to_XYZ(int index, int width);

int XYZ_to_index(coords xyz, int width);

vertex Rotate(vertex v, angles a);

#endif
