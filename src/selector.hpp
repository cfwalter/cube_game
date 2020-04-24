#ifndef SELECTOR_HPP
#define SELECTOR_HPP
#include "cube.hpp"

class Selector {
private:
    int index;
    int old_index;
    bool holding;
    const int move_frames_total = 10;
    int move_frame;
    Cube* cube;
public :
    inline Selector(int i, Cube* c) {index=i; holding=false; cube=c; move_frame=0;};
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline bool is_holding() {return holding;}
    inline void set_holding(bool h) {holding=h;}
    direction move(direction dir);
    void draw(SDL_Renderer * renderer, SDL_Texture* img);
};

void Selector::draw(SDL_Renderer * renderer, SDL_Texture* img)
{
    int center_x = WINDOW_WIDTH / 2;
    int center_y = WINDOW_HEIGHT / 2;
    vertex vert = this->cube->index_to_vertex(this->index);
    if (this->move_frame) {
        vertex old_vert = this->cube->index_to_vertex(this->old_index);
        float dist = 1 - float(this->move_frame) / this->move_frames_total;
        vert.x -= (vert.x - old_vert.x) * dist;
        vert.y -= (vert.y - old_vert.y) * dist;
        vert.z -= (vert.z - old_vert.z) * dist;
        this->move_frame += 1;
    }
    if (this->move_frame == this->move_frames_total) { this->move_frame = 0; }

    int u = FOV*vert.x/vert.z+center_x;
    int v = FOV*vert.y/vert.z+center_y;
    int w, h;
    SDL_QueryTexture(img, NULL, NULL, &w, &h);
    SDL_Rect r = {int(u-w/vert.z*0.5), int(v-w/vert.z*0.5), int(w/vert.z), int(h/vert.z)};
    SDL_RenderCopy(renderer, img, NULL, &r);
}

direction Selector::move(direction dir)
{
    if (this->move_frame) {return direction::null;}  //don't collect input during move animation
    // get rotations of x,y,z with current angle, to see which way is up/down and left/right
    const angles a = this->cube->get_heading_rads();
    vertex rot_x = Rotate({1,0,0}, a);
    vertex rot_y = Rotate({0,1,0}, a);
    vertex rot_z = Rotate({0,0,1}, a);
    axis u_axis, v_axis; // u=left/right, v=up/down
    int lft_u, rgt_u, top_v, bot_v;
    const int width = this->cube->get_width();
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
    this->old_index = this->index;
    this->index = XYZ_to_index(curr_xyz, width);
    this->move_frame = 1;
    return direction::null;
}

#endif
