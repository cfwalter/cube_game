#ifndef TILE_HPP
#define TILE_HPP
#include "main.hpp"

class Cube;

class Tile {
protected:
    int index;
    static SDL_Surface* img_surface;
    SDL_Texture* img_texture;
    SDL_Renderer * rend;
    Cube* cube;
    static const int h = 18;
    static const int w = 18;
public:
    static const TILE_TYPE type = TILE_TYPE::TT_OPEN_TILE;
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    void draw(double x, double y, double z);
    inline Tile(int i, Cube* c, SDL_Renderer * r)
    {
        index=i;
        rend=r;
        cube=c;
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    }
    inline virtual bool is_walkable(int type) {return true;};
    inline virtual void on_exit() {};
    inline virtual bool is_win() {return true;};
};


class OpenTile : public Tile {
protected:
    static SDL_Surface* img_surface;
public:
    static const TILE_TYPE type = TILE_TYPE::TT_OPEN_TILE;
    OpenTile(int i, Cube* c, SDL_Renderer * r) : Tile(i, c, r)
    {
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
    inline bool is_walkable(int type) override {return true;};
};


class WallTile : public Tile {
protected:
    static SDL_Surface* img_surface;
public:
    static const TILE_TYPE type = TILE_TYPE::TT_WALL_TILE;
    WallTile(int i, Cube* c, SDL_Renderer * r) : Tile(i, c, r)
    {
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
    inline bool is_walkable(int type) override {return false;};
};


class PointerOnlyTile : public Tile {
protected:
    static SDL_Surface* img_surface;
public:
    static const TILE_TYPE type = TILE_TYPE::TT_POINTER_ONLY_TILE;
    PointerOnlyTile(int i, Cube* c, SDL_Renderer * r) : Tile(i, c, r)
    {
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
    inline bool is_walkable(int type) override {return type == GAMEPLAY_OBJ_TYPE::GOT_SELECTOR;};
};


class BlockOnlyTile : public Tile {
protected:
    static SDL_Surface* img_surface;
public:
    static const TILE_TYPE type = TILE_TYPE::TT_BLOCK_ONLY_TILE;
    BlockOnlyTile(int i, Cube* c, SDL_Renderer * r) : Tile(i, c, r)
    {
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
    inline bool is_walkable(int type) override {return type == GAMEPLAY_OBJ_TYPE::GOT_BLOCK;};
};


class EmptyTile : public Tile {
public:
    static const TILE_TYPE type = TILE_TYPE::TT_EMPTY_TILE;
    EmptyTile(int i, Cube* c, SDL_Renderer * r) : Tile(i, c, r)
    {
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
    inline bool is_walkable(int type) override {return false;};
};

class FinishTile : public Tile {
protected:
    static SDL_Surface* img_surface;
public:
    static const TILE_TYPE type = TILE_TYPE::TT_FINISH_TILE;
    FinishTile(int i, Cube* c, SDL_Renderer * r) : Tile(i, c, r)
    {
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
    inline bool is_walkable(int type) override {return true;};
    virtual bool is_win() override;
};

// class YellowTile : public Tile {
// protected:
//     static SDL_Surface* img_surface;
//     static SDL_Surface* red_img_surface;
// public:
//     YellowTile(int i, SDL_Renderer * r) : Tile(i, c, r)
//     {
//         solid = false;
//         img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
//     };
//     void on_exit() override
//     {
//         this->solid = true;
//         img_texture = SDL_CreateTextureFromSurface(rend, red_img_surface);
//     };
// };

#endif
