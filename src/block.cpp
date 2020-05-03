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

void Block::draw(vertex current, vertex old)
{
    if (this->move_frame) {
        float dist = 1 - float(this->move_frame) / this->move_frames_total;
        current.x -= (current.x - old.x) * dist;
        current.y -= (current.y - old.y) * dist;
        current.z -= (current.z - old.z) * dist;
    }
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
    if (current.z <= 0) return;
    int u = FOV*current.x/current.z+CENTER_X;
    int v = FOV*current.y/current.z+CENTER_Y;
    int w, h;
    SDL_QueryTexture(img, NULL, NULL, &w, &h);
    double wz = w/current.z;
    double hz = h/current.z;
    SDL_Rect r = {int(u-wz*0.5), int(v-hz*0.5), int(wz), int(hz)};
    SDL_RenderCopy(this->rend, img, NULL, &r);
}

bool Block::move(int dx, int dy, int dz)
{
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

    Tile* next_tile = this->cube->get_tile_at(next_index);
    if (!next_tile->is_walkable(GAMEPLAY_OBJ_TYPE::GOT_BLOCK)) return false;
    Block* next_block = this->cube->get_block_at(next_index);

    if (!next_block || next_block->move(dx, dy, dz)) {
        this->old_index = this->index;
        this->index = next_index;
        this->move_frame = 1;
        return true;
    }
    return false;
}
