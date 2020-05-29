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
    static const TILE_TYPE type = TILE_TYPE::TT_OPEN_TILE;
public:
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
    inline virtual TILE_TYPE get_type() { return this->type; };
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
    inline virtual TILE_TYPE get_type() override { return this->type; };
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
    inline virtual TILE_TYPE get_type() override { return this->type; };
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
    inline virtual TILE_TYPE get_type() override { return this->type; };
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
    inline virtual TILE_TYPE get_type() override { return this->type; };
};


class EmptyTile : public Tile {
public:
    static const TILE_TYPE type = TILE_TYPE::TT_EMPTY_TILE;
    EmptyTile(int i, Cube* c, SDL_Renderer * r) : Tile(i, c, r)
    {
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    };
    inline bool is_walkable(int type) override {return false;};
    inline virtual TILE_TYPE get_type() override { return this->type; };
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
    inline virtual TILE_TYPE get_type() override { return this->type; };
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
