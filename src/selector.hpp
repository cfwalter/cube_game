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
    int hold_frame;
    static SDL_Surface* closed_box_surface;
    SDL_Texture* closed_box_texture;
    static SDL_Surface* open_box_surface;
    SDL_Texture* open_box_texture;
    SDL_Renderer * rend;
    Cube* cube;
public :
    inline Selector(SDL_Renderer * r, int i, Cube* c)
    {
        index=i; holding=false; cube=c; move_frame=0; hold_frame=0;
        rend = r;
        closed_box_texture = SDL_CreateTextureFromSurface(rend, closed_box_surface);
        open_box_texture = SDL_CreateTextureFromSurface(rend, open_box_surface);
    };
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline bool is_holding() {return holding;}
    inline void set_holding(bool h) {holding=h;}
    inline void toggle_holding() {holding=!holding;}
    direction move(direction dir);
    void draw();
};

SDL_Surface* Selector::open_box_surface = IMG_Load("Resources/open_box.png");
SDL_Surface* Selector::closed_box_surface = IMG_Load("Resources/closed_box.png");

void Selector::draw()
{
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

    int u = FOV*vert.x/vert.z+CENTER_X;
    int v = FOV*vert.y/vert.z+CENTER_Y;
    int w, h;
    SDL_Texture* tex = (this->holding? this->closed_box_texture : this->open_box_texture);
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect r = {int(u-w/vert.z*0.5), int(v-w/vert.z*0.5), int(w/vert.z), int(h/vert.z)};
    SDL_RenderCopy(this->rend, tex, NULL, &r);
}

direction Selector::move(direction dir)
{
    if (this->move_frame) {return direction::null;}  //don't collect input during move animation

    // Cube logic
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

    // Tile logic
    int i = XYZ_to_index(curr_xyz, width);
    if (this->cube->get_edit_mode()) { // in edit mode, ignore all tile logic
        this->old_index = this->index;
        this->index = i;
        this->move_frame = 1;
        return direction::null;
    }
    if (this->cube->get_tile_at(i)->is_walkable()) {
        this->old_index = this->index;
        this->index = i;
        this->move_frame = 1;
    } else {
        this->old_index = i;
        this->move_frame = this->move_frames_total / 2;
    }
    return direction::null;
}

#endif
