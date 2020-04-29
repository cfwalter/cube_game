#ifndef TILE_HPP
#define TILE_HPP
#include "main.hpp"

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

#endif
