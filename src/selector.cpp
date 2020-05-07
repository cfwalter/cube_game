#include "common.hpp"
#include "selector.hpp"

SDL_Surface* Selector::open_surface = IMG_Load("../Resources/point_hand.png");
SDL_Surface* Selector::closed_surface = IMG_Load("../Resources/pinch_hand.png");

SDL_Surface* LinkedBlockEditor::selected_surface = IMG_Load("../Resources/selected.png");


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
    const int max = this->cube->get_width()-1;
    coords curr_xyz = Index_to_XYZ(index, this->cube->get_width());
    coords next_xyz = this->cube->get_next_coords(axis::AXIS_X, axis::AXIS_Y, dir, this->index);

    bool max_x = next_xyz.x > max; bool min_x = next_xyz.x < 0;
    bool max_y = next_xyz.y > max; bool min_y = next_xyz.y < 0;
    bool max_z = next_xyz.z > max; bool min_z = next_xyz.z < 0;
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
    int dx = next_xyz.x - curr_xyz.x;
    int dy = next_xyz.y - curr_xyz.y;
    int dz = next_xyz.z - curr_xyz.z;

    // Tile logic
    int new_index = XYZ_to_index(next_xyz, this->cube->get_width());
    if (this->cube->get_edit_mode()) { // in edit mode, ignore all tile logic
        this->old_index = this->index;
        this->index = new_index;
        this->move_frame = 1;
        return direction::null;
    }
    bool tile_success = this->cube->get_tile_at(new_index)->is_walkable(GAMEPLAY_OBJ_TYPE::GOT_SELECTOR);
    bool block_success = true;

    // Block logic
    if (tile_success && this->held_block) {
        block_success = this->held_block->move(dir);
    }

    if (tile_success && block_success) {
        this->cube->get_tile_at(this->index)->on_exit();
        this->old_index = this->index;
        this->index = new_index;
        this->move_frame = 1;
    } else {
        this->old_index = new_index;
        this->move_frame = this->move_frames_total / 2;
    }
    return direction::null;
}
