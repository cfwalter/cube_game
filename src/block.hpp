#ifndef BLOCK_HPP
#define BLOCK_HPP
#include "main.hpp"
#include "common.hpp"

class Cube;

class Block {
protected:
    int index;
    int old_index;
    int move_frame;
    CUBE_FACE face;
    static const int move_frames_total = 10;
    static const bool mobile = true;
    static SDL_Surface* front_surface;
    static SDL_Surface*  left_surface;
    static SDL_Surface* right_surface;
    static SDL_Surface*    up_surface;
    static SDL_Surface*  down_surface;
    SDL_Texture* front_texture;
    SDL_Texture*  left_texture;
    SDL_Texture* right_texture;
    SDL_Texture*    up_texture;
    SDL_Texture*  down_texture;
    SDL_Renderer * rend;
    Cube* cube;
public:
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline Block(int i, CUBE_FACE f, Cube* c, SDL_Renderer * r)
    {
        index=i;
        old_index=i;
        face = f;
        cube=c;
        rend=r;
        move_frame=0;
        front_texture = SDL_CreateTextureFromSurface(rend, front_surface);
        left_texture = SDL_CreateTextureFromSurface(rend, left_surface);
        right_texture = SDL_CreateTextureFromSurface(rend, right_surface);
        up_texture = SDL_CreateTextureFromSurface(rend, up_surface);
        down_texture = SDL_CreateTextureFromSurface(rend, down_surface);
    }
    inline bool is_mobile() {return mobile;};
    inline void set_old_index(int v) {old_index = v;};
    inline int  get_old_index() {return old_index;};
    inline void start_move() {move_frame = 1;};
    void set_face(CUBE_FACE f) {this->face = f;}
    bool move(int dx, int dy, int dz);
    void update();
    void draw(vertex current, vertex old);
};

#endif
