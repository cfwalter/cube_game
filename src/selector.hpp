#ifndef SELECTOR_HPP
#define SELECTOR_HPP
#include "main.hpp"
#include "cube.hpp"

class Selector {
private:
    int index;
    int old_index;
    bool holding;
    const int move_frames_total = 10;
    int move_frame;
    int hold_frame;
    static SDL_Surface* closed_box_surface;
    SDL_Texture* closed_box_texture;
    static SDL_Surface* open_box_surface;
    SDL_Texture* open_box_texture;
    SDL_Renderer * rend;
    Cube* cube;
    Block* held_block;
public :
    inline Selector(SDL_Renderer * r, int i, Cube* c)
    {
        index=i; holding=false; cube=c; move_frame=0; hold_frame=0;
        held_block = NULL;
        rend = r;
        closed_box_texture = SDL_CreateTextureFromSurface(rend, closed_box_surface);
        open_box_texture = SDL_CreateTextureFromSurface(rend, open_box_surface);
    };
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline bool is_holding() {return holding;}
    inline void set_holding(bool h) {holding=h; set_held_block();}
    inline void set_held_block()
    {
        if (this->holding) {
            this->held_block = this->cube->get_block_at(this->index);
        } else {
            this->held_block = NULL;
        }
    }
    inline void toggle_holding() {holding=!holding; set_held_block();}
    direction move(direction dir);
    void update();
    void draw();
};

#endif
