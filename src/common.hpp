#ifndef COMMON_HPP
#define COMMON_HPP

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

#endif
