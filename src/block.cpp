#include "block.hpp"
#include "cube.hpp"


SDL_Surface* Block::img_surface = IMG_Load("Resources/pink_diamond.png");

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
    if (current.z <= 0) return;
    int u = FOV*current.x/current.z+CENTER_X;
    int v = FOV*current.y/current.z+CENTER_Y;
    int w, h;
    SDL_QueryTexture(img_texture, NULL, NULL, &w, &h);
    double wz = w/current.z;
    double hz = h/current.z;
    SDL_Rect r = {int(u-wz*0.5), int(v-hz*0.5), int(wz), int(hz)};
    SDL_RenderCopy(this->rend, img_texture, NULL, &r);
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
    if (out_x || out_y || out_z) {
        return false;
    }

    int next_index = XYZ_to_index(next_xyz, this->cube->get_width());

    // TODO: check tile at next index, return tile.is_walkable
    Block* next_block = this->cube->get_block_at(next_index);
    // if () return true;
    if (!next_block || next_block->move(dx, dy, dz)) {
        this->old_index = this->index;
        this->index = next_index;
        this->move_frame = 1;
        return true;
    }
    return false;
}
