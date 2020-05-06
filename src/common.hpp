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
    AXIS_X=0,
    AXIS_Y,
    AXIS_Z,
    AXIS_NONE,
};

enum direction
{
    up=0,
    right,
    down,
    left,
    null,
};

enum GAMEPLAY_OBJ_TYPE
{
    GOT_SELECTOR=0,
    GOT_BLOCK,
    GOT_TILE,
};

enum TILE_TYPE
{
    TT_OPEN_TILE=1,
    TT_WALL_TILE,
    TT_POINTER_ONLY_TILE,
    TT_BLOCK_ONLY_TILE,
    TT_EMPTY_TILE,
};

enum CUBE_FACE
{
    MAX_X,
    MIN_X,
    MAX_Y,
    MIN_Y,
    MAX_Z,
    MIN_Z,
};

float DegToRad(int deg);

float RadToDeg(float rad);

void AddAxis(coords* xyz, int n, axis a);

double get_axis(vertex xyz, axis a);

coords Index_to_XYZ(int index, int width);

int XYZ_to_index(coords xyz, int width);

vertex Rotate(vertex v, angles a);

#endif
