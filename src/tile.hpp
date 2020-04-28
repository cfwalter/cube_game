#ifndef TILE_HPP
#define TILE_HPP
// do not #include "cube.hpp"

class Tile {
protected:
    int index;
    bool solid = false;
    static SDL_Surface* img_surface;
    SDL_Texture* img_texture;
    SDL_Renderer * rend;
    static const int h = 18;
    static const int w = 18;
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
    inline bool is_walkable() {return !this->solid;};
    inline virtual void on_exit() {};
};

SDL_Surface* Tile::img_surface = IMG_Load("Resources/blu_ring.png");

void Tile::draw(double x, double y, double z)
{
    int u = FOV*x/z+CENTER_X;
    int v = FOV*y/z+CENTER_Y;
    // int w, h;
    // SDL_QueryTexture(img_texture, NULL, NULL, &w, &h);
    double wz = w/z;
    double hz = h/z;
    SDL_Rect r = {int(u-wz*0.5), int(v-hz*0.5), int(wz), int(hz)};
    SDL_RenderCopy(this->rend, img_texture, NULL, &r);
}

class RedTile : public Tile {
protected:
    static SDL_Surface* img_surface;
public:
    RedTile(int i, SDL_Renderer * r) : Tile(i, r)
    {
        solid = true;
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
};

SDL_Surface* RedTile::img_surface = IMG_Load("Resources/red_ring.png");

class YellowTile : public Tile {
protected:
    static SDL_Surface* img_surface;
    static SDL_Surface* red_img_surface;
public:
    YellowTile(int i, SDL_Renderer * r) : Tile(i, r)
    {
        solid = false;
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
    void on_exit() override
    {
        this->solid = true;
        img_texture = SDL_CreateTextureFromSurface(rend, red_img_surface);
    };
};

SDL_Surface* YellowTile::img_surface = IMG_Load("Resources/yellow_ring.png");
SDL_Surface* YellowTile::red_img_surface = IMG_Load("Resources/red_ring.png");

#endif
