#include "block.hpp"
#include "tile.hpp"
#include "cube.hpp"


SDL_Surface* Block::front_surface = IMG_Load("Resources/pink_diamond.png");
SDL_Surface* Block::left_surface  = IMG_Load("Resources/pink_diamond_left.png");
SDL_Surface* Block::right_surface = IMG_Load("Resources/pink_diamond_right.png");
SDL_Surface* Block::up_surface    = IMG_Load("Resources/pink_diamond_up.png");
SDL_Surface* Block::down_surface  = IMG_Load("Resources/pink_diamond_down.png");

void Block::update()
{
    if (this->move_frame) { this->move_frame += 1; }
    if (this->move_frame == this->move_frames_total) { this->move_frame = 0; }
}


vertex Block::get_current_vertex() {
    vertex current_vert = this->cube->index_to_vertex(this->index);
    vertex old_vert = this->cube->index_to_vertex(this->old_index);
    if (this->move_frame) {
        float dist = 1 - float(this->move_frame) / this->move_frames_total;
        current_vert.x -= (current_vert.x - old_vert.x) * dist;
        current_vert.y -= (current_vert.y - old_vert.y) * dist;
        current_vert.z -= (current_vert.z - old_vert.z) * dist;
    }
    return current_vert;
}


void Block::draw()
{
    vertex current_vert = this->get_current_vertex();
    SDL_Texture* img;
    if (this->cube->is_heading_square()){
        vertex unit_v;
        if (this->face == MAX_X) unit_v = { 1,  0,  0};
        if (this->face == MIN_X) unit_v = {-1,  0,  0};
        if (this->face == MAX_Y) unit_v = { 0,  1,  0};
        if (this->face == MIN_Y) unit_v = { 0, -1,  0};
        if (this->face == MAX_Z) unit_v = { 0,  0,  1};
        if (this->face == MIN_Z) unit_v = { 0,  0, -1};
        vertex rot_v = Rotate(unit_v, this->cube->get_heading_rads());

        if (rot_v.x== 1) img = this->right_texture;
        if (rot_v.x==-1) img = this->left_texture;
        if (rot_v.y== 1) img = this->down_texture;
        if (rot_v.y==-1) img = this->up_texture;
        if (rot_v.z== 1) img = this->front_texture;
        if (rot_v.z==-1) img = this->front_texture;
    } else {
        img = this->front_texture;
    }
    if (current_vert.z <= 0) return;
    int u = FOV*current_vert.x/current_vert.z+CENTER_X;
    int v = FOV*current_vert.y/current_vert.z+CENTER_Y;
    int w, h;
    SDL_QueryTexture(img, NULL, NULL, &w, &h);
    double wz = w/current_vert.z;
    double hz = h/current_vert.z;
    SDL_Rect r = {int(u-wz*0.5), int(v-hz*0.5), int(wz), int(hz)};
    SDL_RenderCopy(this->rend, img, NULL, &r);
    vertex lv;
    for (int i=0; i<this->linked_blocks.size(); ++i) {
        lv = linked_blocks.at(i)->get_current_vertex();
        if (lv.z > 0) {
            SDL_RenderDrawLine(this->rend, u, v, FOV*lv.x/lv.z+CENTER_X, FOV*lv.y/lv.z+CENTER_Y);
        }
    }
}


bool Block::move(int dx, int dy, int dz)
{
    if (this->moved_already) {
        return true;
    }
    this->moved_already = true;  // already_moved is reset after each frame, ensure that it won't be moved more than once
    coords curr_xyz = Index_to_XYZ(this->index, this->cube->get_width());
    coords next_xyz;
    next_xyz.x = curr_xyz.x + dx;
    next_xyz.y = curr_xyz.y + dy;
    next_xyz.z = curr_xyz.z + dz;

    const int max = this->cube->get_width()-1;
    bool out_x = next_xyz.x < 0 || next_xyz.x > max;
    bool out_y = next_xyz.y < 0 || next_xyz.y > max;
    bool out_z = next_xyz.z < 0 || next_xyz.z > max;
    if (out_x || out_y || out_z) return false;

    int next_index = XYZ_to_index(next_xyz, this->cube->get_width());

    // 1. Check if tile is walkable
    Tile* next_tile = this->cube->get_tile_at(next_index);
    if (!next_tile->is_walkable(GAMEPLAY_OBJ_TYPE::GOT_BLOCK)) return false;

    // 2. push the blocks in front
    Block* next_block = this->cube->get_block_at(next_index);
    if (next_block && !next_block->move(dx, dy, dz)) {
        return false;
    }
    this->old_index = this->index;
    this->index = next_index;
    this->move_frame = 1;

    // 3. attempt to move linked blocks
    for (int i=0; i<this->linked_blocks.size(); ++i) {
        this->linked_blocks.at(i)->move(dx, dy, dz);
    }

    return true;
}
