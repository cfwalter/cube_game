#ifndef CUBE_HPP
#define CUBE_HPP
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

void AddAxis(coords* xyz, int n, axis a)
{
    if (a==axis::x) xyz->x += n;
    if (a==axis::y) xyz->y += n;
    if (a==axis::z) xyz->z += n;
}

float DegToRad(int deg)
{
    return (deg*PI/180.0);
}

float RadToDeg(float rad)
{
    return (rad/PI*180);
}

class Cube {
private:
    vertex origin;
    int width;
    angles heading;
    angles target_heading;
public:
    inline Cube(vertex o, int w, angles h, angles th)
    {
        origin=o; width=w; heading=h; target_heading=th;
    };
    inline vertex get_origin() {return origin;};
    inline int get_width() {return width;};
    inline angles get_heading_degs() {return heading;};
    inline angles get_heading_rads()
    {
        return {DegToRad(heading.phi), DegToRad(heading.theta), DegToRad(heading.psi)};
    };
    inline void set_heading_degs(angles h) {heading=h;};
    inline void set_heading_rads(angles h)
    {
        heading={RadToDeg(h.phi), RadToDeg(h.theta), RadToDeg(h.psi)};
    };
    inline bool is_heading_square()
    {
        return (!(int(heading.psi)%90) && !(int(heading.theta)%90) && !(int(heading.phi)%90));
    };
    void rotate(direction dir);
    void update();
    void draw(SDL_Renderer * renderer, SDL_Texture* img);
};

void Cube::rotate(direction dir)
{
    // Logic here to make sure cube always rotates relative to the camera
    // TODO: fix edge case, possibly replace with logic table
    if (dir == direction::up || dir == direction::down) {
        int d = 90 * (dir == direction::down ? 1 : -1);
        int deg = (int(heading.theta)%360+360)%360;
        switch (deg/90){
            case 0: target_heading.phi -= d; break;
            case 1: target_heading.psi -= d; break;
            case 2: target_heading.phi -= d; break;
            case 3: target_heading.psi += d; break;
        }
    }
    if (dir == direction::left || dir == direction::right) {
        int d = 90 * (dir == direction::left ? 1 : -1);
        int deg = (int(heading.phi)%360+360)%360;
        switch (deg/90){
            case 0: target_heading.theta -= d; break;
            case 1: target_heading.psi += d; break;
            case 2: target_heading.theta += d; break;
            case 3: target_heading.psi -= d; break;
        }
    }
}

void Cube::update()
{
    // TODO: make this pythonic idk
    if (heading.psi   < target_heading.psi)   heading.psi   += D_ANGLE;
    if (heading.psi   > target_heading.psi)   heading.psi   -= D_ANGLE;
    if (heading.phi   < target_heading.phi)   heading.phi   += D_ANGLE;
    if (heading.phi   > target_heading.phi)   heading.phi   -= D_ANGLE;
    if (heading.theta < target_heading.theta) heading.theta += D_ANGLE;
    if (heading.theta > target_heading.theta) heading.theta -= D_ANGLE;
}

void Cube::draw(SDL_Renderer * renderer, SDL_Texture* img)
{
  int center_x = WINDOW_WIDTH / 2;
  int center_y = WINDOW_HEIGHT / 2;
  float x,y,z;
  vertex o = origin;
  angles a = this->get_heading_rads();

  const double w = 1.0;
  double space = w/(width-1); // 1/nw * w
  double half_width = w/2;
  for (int i=0; i<pow(width,3); ++i) {
    coords xyz = Index_to_XYZ(i, width);
    // TODO: gotta be a better way to do this logic
    bool front_face = !(xyz.x && xyz.y && xyz.z);
    bool back_face  = !((xyz.x+1)%width && (xyz.y+1)%width && (xyz.z+1)%width);
    if ( front_face || back_face){ //test if
      x = xyz.x * space - half_width;
      y = xyz.y * space - half_width;
      z = xyz.z * space - half_width;
      vertex v = Rotate({x, y, z}, a);
      x=v.x+o.x; y=v.y+o.y; z=v.z+o.z;

      if (z>0) {
        int u = FOV*x/z+center_x; int v = FOV*y/z+center_y;
        int w, h;
        SDL_QueryTexture(img, NULL, NULL, &w, &h);
        w = w/z; h = h/z;
        SDL_Rect r = {int(u-w*0.5), int(v-w*0.5), int(w), int(h)};
        SDL_RenderCopy(renderer, img, NULL, &r);
      }
    }
  }
}

class Selector {
private:
    int index;
    bool holding;
public :
    inline Selector(int i) {index=i; holding=false;};
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline bool is_holding() {return holding;}
    inline void set_holding(bool h) {holding=h;}
    void render() const;
    direction move(angles a, direction dir, int width);
    void draw(SDL_Renderer * renderer, vertex o, angles a, int cube_w, SDL_Texture* img);
};

void Selector::draw(SDL_Renderer * renderer, vertex o, angles a, int cube_w, SDL_Texture* img)
{
  // TODO: remove duplicated logic with DrawCube
  int center_x = WINDOW_WIDTH / 2;
  int center_y = WINDOW_HEIGHT / 2;
  float x,y,z;

  const double width = 1.0;
  double space = width/(cube_w-1); // 1/nw * width
  double half_width = width/2;
  coords xyz = Index_to_XYZ(index, cube_w);
  x = xyz.x * space - half_width;
  y = xyz.y * space - half_width;
  z = xyz.z * space - half_width;
  vertex vert = Rotate({x, y, z}, a);
  x=vert.x+o.x; y=vert.y+o.y; z=vert.z+o.z;

  int u = FOV*x/z+center_x; int v = FOV*y/z+center_y;
  int w, h;
  SDL_QueryTexture(img, NULL, NULL, &w, &h);
  SDL_Rect r = {int(u-w/z*0.5), int(v-w/z*0.5), int(w/z), int(h/z)};
  SDL_RenderCopy(renderer, img, NULL, &r);
}

direction Selector::move(angles a, direction dir, int width)
{
    // get rotations of x,y,z with current angle, to see which way is up/down and left/right
    vertex rot_x = Rotate({1,0,0}, a);
    vertex rot_y = Rotate({0,1,0}, a);
    vertex rot_z = Rotate({0,0,1}, a);
    axis u_axis, v_axis; // u=left/right, v=up/down
    int lft_u, rgt_u, top_v, bot_v;
    const int max = width-1;

    if (rot_x.x== 1) {u_axis=axis::x; lft_u=0;   rgt_u=max;}
    if (rot_x.x==-1) {u_axis=axis::x; lft_u=max; rgt_u=0;}
    if (rot_y.x== 1) {u_axis=axis::y; lft_u=0;   rgt_u=max;}
    if (rot_y.x==-1) {u_axis=axis::y; lft_u=max; rgt_u=0;}
    if (rot_z.x== 1) {u_axis=axis::z; lft_u=0;   rgt_u=max;}
    if (rot_z.x==-1) {u_axis=axis::z; lft_u=max; rgt_u=0;}

    if (rot_x.y== 1) {v_axis=axis::x; top_v=0;   bot_v=max;}
    if (rot_x.y==-1) {v_axis=axis::x; top_v=max; bot_v=0;}
    if (rot_y.y== 1) {v_axis=axis::y; top_v=0;   bot_v=max;}
    if (rot_y.y==-1) {v_axis=axis::y; top_v=max; bot_v=0;}
    if (rot_z.y== 1) {v_axis=axis::z; top_v=0;   bot_v=max;}
    if (rot_z.y==-1) {v_axis=axis::z; top_v=max; bot_v=0;}

    int du = (rgt_u - lft_u) / (max);
    int dv = (bot_v - top_v) / (max);

    coords curr_xyz = Index_to_XYZ(index, width);
    switch(dir) {
        case direction::up:    AddAxis(&curr_xyz, -dv, v_axis); break;
        case direction::down:  AddAxis(&curr_xyz,  dv, v_axis); break;
        case direction::left:  AddAxis(&curr_xyz, -du, u_axis); break;
        case direction::right: AddAxis(&curr_xyz,  du, u_axis); break;
        case direction::null: break;
    }
    bool out_x = curr_xyz.x < 0 || curr_xyz.x > max;
    bool out_y = curr_xyz.y < 0 || curr_xyz.y > max;
    bool out_z = curr_xyz.z < 0 || curr_xyz.z > max;
    if (out_x || out_y || out_z) {
        return dir;
    }
    index = XYZ_to_index(curr_xyz, width);
    return direction::null;
}


#endif
