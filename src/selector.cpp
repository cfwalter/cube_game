#include "common.hpp"
#include "selector.hpp"

SDL_Surface* Selector::open_surface = IMG_Load("Resources/point_hand.png");
SDL_Surface* Selector::closed_surface = IMG_Load("Resources/pinch_hand.png");


void Selector::update()
{
    if (this->move_frame) { this->move_frame += 1; }
    if (this->move_frame == this->move_frames_total) { this->move_frame = 0; }
}


void Selector::draw()
{
    vertex vert = this->cube->index_to_vertex(this->index);
    if (this->move_frame) {
        vertex old_vert = this->cube->index_to_vertex(this->old_index);
        float dist = 1 - float(this->move_frame) / this->move_frames_total;
        vert.x -= (vert.x - old_vert.x) * dist;
        vert.y -= (vert.y - old_vert.y) * dist;
        vert.z -= (vert.z - old_vert.z) * dist;
    }

    int u = FOV*vert.x/vert.z+CENTER_X;
    int v = FOV*vert.y/vert.z+CENTER_Y;
    int w, h;
    SDL_Texture* tex = (this->is_holding()? this->closed_texture : this->open_texture);
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect r = {u-20, v-20, 20, 20};
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
    int dx, dy, dz;

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
    coords next_xyz = curr_xyz;  // keep track of original, but alter the new
    switch(dir) {
        case direction::up:    AddAxis(&next_xyz, -dv, v_axis); break;
        case direction::down:  AddAxis(&next_xyz,  dv, v_axis); break;
        case direction::left:  AddAxis(&next_xyz, -du, u_axis); break;
        case direction::right: AddAxis(&next_xyz,  du, u_axis); break;
        case direction::null: break;
    }
    bool max_x = next_xyz.x > max;
    bool max_y = next_xyz.y > max;
    bool max_z = next_xyz.z > max;
    bool min_x = next_xyz.x < 0;
    bool min_y = next_xyz.y < 0;
    bool min_z = next_xyz.z < 0;
    bool out_x = min_x || max_x;
    bool out_y = min_y || max_y;
    bool out_z = min_z || max_z;
    if (out_x || out_y || out_z) {
        if (this->held_block) {
            if (max_x) this->held_block->set_face(CUBE_FACE::MAX_X);
            if (max_y) this->held_block->set_face(CUBE_FACE::MAX_Y);
            if (max_z) this->held_block->set_face(CUBE_FACE::MAX_Z);
            if (min_x) this->held_block->set_face(CUBE_FACE::MIN_X);
            if (min_y) this->held_block->set_face(CUBE_FACE::MIN_Y);
            if (min_z) this->held_block->set_face(CUBE_FACE::MIN_Z);
        }
        return dir;
    }

    // TODO:
    dx = next_xyz.x - curr_xyz.x;
    dy = next_xyz.y - curr_xyz.y;
    dz = next_xyz.z - curr_xyz.z;

    // Tile logic
    int i = XYZ_to_index(next_xyz, width);
    if (this->cube->get_edit_mode()) { // in edit mode, ignore all tile logic
        this->old_index = this->index;
        this->index = i;
        this->move_frame = 1;
        return direction::null;
    }
    bool tile_success = this->cube->get_tile_at(i)->is_walkable(GAMEPLAY_OBJ_TYPE::GOT_SELECTOR);
    bool block_success = true;

    // Block logic
    if (tile_success && this->held_block) {
        block_success = this->held_block->move(dx, dy, dz);
    }

    if (tile_success && block_success) {
        this->cube->get_tile_at(this->index)->on_exit();
        this->old_index = this->index;
        this->index = i;
        this->move_frame = 1;
    } else {
        this->old_index = i;
        this->move_frame = this->move_frames_total / 2;
    }
    return direction::null;
}
