#ifndef BLOCK_HPP
#define BLOCK_HPP
#include "common.hpp"
// do not #include "cube.hpp"

class Block {
protected:
    int index;
    int old_index;
    int move_frame;
    static const int move_frames_total = 10;
    static const bool mobile = true;
    static SDL_Surface* img_surface;
    SDL_Texture* img_texture;
    SDL_Renderer * rend;
public:
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline Block(int i, SDL_Renderer * r)
    {
        index=i;
        old_index=i;
        rend=r;
        move_frame=0;
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    }
    inline bool is_mobile() {return mobile;};
    inline void set_old_index(int v) {old_index = v;};
    inline int  get_old_index() {return old_index;};
    inline void start_move() {move_frame = 1;};
    void update();
    void draw(vertex current, vertex old);
};

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

#endif
