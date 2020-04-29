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

bool Block::move(int index)
{
    return !this->cube->get_block_at(index);
}
