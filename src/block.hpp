#ifndef BLOCK_HPP
#define BLOCK_HPP
#include "common.hpp"
// do not #include "cube.hpp"

class Block {
protected:
    int index;
    static const bool mobile = true;
    static SDL_Surface* img_surface;
    SDL_Texture* img_texture;
    SDL_Renderer * rend;
    static const int move_frames_total = 10;
    int move_frame;
    vertex old_vertex;
public:
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline Block(int i, SDL_Renderer * r)
    {
        index=i;
        rend=r;
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    }
    inline bool is_mobile() {return mobile;};
    inline void set_old_vertex(vertex v) {old_vertex = v;};
    inline void start_move() {move_frame = 1;};
    void update();
    void draw(double x, double y, double z);
};

SDL_Surface* Block::img_surface = IMG_Load("Resources/pink_diamond.png");

void Block::update()
{
    if (this->move_frame) { this->move_frame += 1; }
    if (this->move_frame == this->move_frames_total) { this->move_frame = 0; }
}

void Block::draw(double x, double y, double z)
{
    if (this->move_frame) {
        float dist = 1 - float(this->move_frame) / this->move_frames_total;
        x -= (x - old_vertex.x) * dist;
        y -= (y - old_vertex.y) * dist;
        z -= (z - old_vertex.z) * dist;
    }
    int u = FOV*x/z+CENTER_X;
    int v = FOV*y/z+CENTER_Y;
    int w, h;
    SDL_QueryTexture(img_texture, NULL, NULL, &w, &h);
    double wz = w/z;
    double hz = h/z;
    SDL_Rect r = {int(u-wz*0.5), int(v-hz*0.5), int(wz), int(hz)};
    SDL_RenderCopy(this->rend, img_texture, NULL, &r);
}

#endif
