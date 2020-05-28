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
    bool moved_already;
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
        moved_already = false;
        front_texture = SDL_CreateTextureFromSurface(rend, front_surface);
        left_texture = SDL_CreateTextureFromSurface(rend, left_surface);
        right_texture = SDL_CreateTextureFromSurface(rend, right_surface);
        up_texture = SDL_CreateTextureFromSurface(rend, up_surface);
        down_texture = SDL_CreateTextureFromSurface(rend, down_surface);
    }
    inline bool is_mobile() {return mobile;};
    inline void set_old_index(int v) {old_index = v;};
    inline int  get_old_index() {return old_index;}
    inline void start_move() {move_frame = 1;};
    inline void reset_moved_already() {this->moved_already = false;};
    void toggle_linked_block(Block* b);
    vertex get_current_vertex();
    void set_face(CUBE_FACE f) {this->face = f;}
    relative_face get_current_face();
    bool move(direction dir);
    void update();
    void draw();
};


class BlockChain {
private:
    Block* block_a;
    Block* block_b;
    Cube* cube;
    SDL_Renderer * rend;
    int offset_n;
    static const int max_offset_n = 15;
public:
    BlockChain(Block* ba, Block* bb, SDL_Renderer * r) { block_a=ba; block_b=bb; offset_n=0; rend=r;};
    inline void update(const Block* held_block)
    {
        if (held_block == block_a) offset_n = (offset_n + 1) % max_offset_n;
        if (held_block == block_b) offset_n = (offset_n - 1) % max_offset_n;
    };
    void draw();
    Block* get_other_block(Block* b) {
        if (b == block_a) return block_b;
        if (b == block_b) return block_a;
        return NULL;
    }
};

#endif
