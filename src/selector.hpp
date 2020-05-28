#ifndef SELECTOR_HPP
#define SELECTOR_HPP
#include "main.hpp"
#include "cube.hpp"

class Selector {
private:
    int index;
    int old_index;
    const int move_frames_total = 10;
    int move_frame;
    static SDL_Surface* closed_surface;
    SDL_Texture* closed_texture;
    static SDL_Surface* open_surface;
    SDL_Texture* open_texture;
    SDL_Renderer * rend;
    Cube* cube;
    Block* held_block;
public :
    inline Selector(SDL_Renderer * r, int i, Cube* c)
    {
        index=i; cube=c; move_frame=0;
        held_block = NULL;
        rend = r;
        closed_texture = SDL_CreateTextureFromSurface(rend, closed_surface);
        open_texture = SDL_CreateTextureFromSurface(rend, open_surface);
    };
    inline int get_index() {return index;};
    inline void set_index(int i) {index=i;};
    inline bool is_holding() {return bool(held_block);}
    inline void grab_block(){this->held_block = this->cube->get_block_at(this->index);}
    inline void toggle_holding() {
        if(!this->is_holding()) grab_block();
        else this->held_block = NULL;
    }
    inline const Block* get_held_block() { return this->held_block; };
    direction move(direction dir);

    void update();
    void draw();
};

class LinkedBlockEditor {
private:
    Cube* cube;
    Block* selected_block;
    SDL_Renderer * rend;
    static SDL_Surface*  selected_surface;
    SDL_Texture*  selected_texture;
public:
    inline LinkedBlockEditor(SDL_Renderer * r, Cube* c)
    {
        rend=r;
        cube=c;
        selected_block=NULL;
        selected_texture = SDL_CreateTextureFromSurface(rend, selected_surface);
    };
    inline void clear_selection() {this->selected_block=NULL;};
    inline void toggle_selection(Block* new_block) {
        if (!new_block) return;
        if (!this->selected_block) {this->selected_block = new_block; return;}
        if (new_block == this->selected_block) {this->selected_block = NULL; return;}
        this->selected_block->toggle_linked_block(new_block);
        this->selected_block = NULL;
    };
    inline void draw() {
        if (!this->selected_block) return;
        vertex vert = this->cube->index_to_vertex(this->selected_block->get_index());
        if (vert.z <= 0) return;
        int u = FOV*vert.x/vert.z+CENTER_X;
        int v = FOV*vert.y/vert.z+CENTER_Y;
        int w, h;
        SDL_QueryTexture(selected_texture, NULL, NULL, &w, &h);
        double wz = w/vert.z;
        double hz = h/vert.z;
        SDL_Rect r = {int(u-wz*0.5), int(v-hz*0.5), int(wz), int(hz)};
        SDL_RenderCopy(this->rend, selected_texture, NULL, &r);
    }
};

#endif
