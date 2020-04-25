#ifndef TILE_HPP
#define TILE_HPP
// do not #include "cube.hpp"

class Tile {
protected:
    int index;
    static SDL_Surface* img_surface;
    SDL_Texture* img_texture;
    SDL_Renderer * rend;
public:
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    void draw(double x, double y, double z);
    inline Tile(int i, SDL_Renderer * r)
    {
        index=i;
        rend=r;
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    }
};

SDL_Surface* Tile::img_surface = IMG_Load("Resources/blu_ring.png");


void Tile::draw(double x, double y, double z)
{
    int u = FOV*x/z+CENTER_X;
    int v = FOV*y/z+CENTER_Y;
    int w, h;
    SDL_QueryTexture(img_texture, NULL, NULL, &w, &h);
    w = w/z; h = h/z;
    SDL_Rect r = {int(u-w*0.5), int(v-h*0.5), w, h};
    SDL_RenderCopy(this->rend, img_texture, NULL, &r);
}

class RedTile : public Tile {
protected:
    static SDL_Surface* img_surface;
public:
    RedTile(int i, SDL_Renderer * r) : Tile(i, r)
    {
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
};
SDL_Surface* RedTile::img_surface = IMG_Load("Resources/red_ring.png");

#endif
