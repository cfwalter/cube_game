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
    static const int move_frames_total = 10;
    static const bool mobile = true;
    static SDL_Surface* img_surface;
    SDL_Texture* img_texture;
    SDL_Renderer * rend;
    Cube* cube;
public:
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline Block(int i, Cube* c, SDL_Renderer * r)
    {
        index=i;
        old_index=i;
        cube=c;
        rend=r;
        move_frame=0;
        img_texture = SDL_CreateTextureFromSurface(rend, img_surface);
    }
    inline bool is_mobile() {return mobile;};
    inline void set_old_index(int v) {old_index = v;};
    inline int  get_old_index() {return old_index;};
    inline void start_move() {move_frame = 1;};
    bool move(int index);
    void update();
    void draw(vertex current, vertex old);
};

#endif
