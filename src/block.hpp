#ifndef BLOCK_HPP
#define BLOCK_HPP
// do not #include "cube.hpp"

class Block {
protected:
    int index;
    static const bool mobile = true;
    static SDL_Surface* img_surface;
    SDL_Texture* img_texture;
    SDL_Renderer * rend;
public:
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    void draw(double x, double y, double z);
    inline Block(int i, SDL_Renderer * r)
    {
        index=i;
        rend=r;
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    }
    inline bool is_mobile() {return mobile;};
};

SDL_Surface* Block::img_surface = IMG_Load("Resources/pink_diamond.png");


void Block::draw(double x, double y, double z)
{
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
