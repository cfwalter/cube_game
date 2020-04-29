#ifndef CUBE_HPP
#define CUBE_HPP
#include "main.hpp"
#include "tile.hpp"
#include "block.hpp"
#include "quat.hpp"
#include "common.hpp"

class Cube {
private:
    vertex origin;
    int width;
    bool edit_mode = false;
    angles heading;
    angles target_heading;
    SDL_Renderer * rend;
    std::vector <Tile*> tiles;
    std::vector <Block*> blocks;
public:
    inline Cube(SDL_Renderer * r, vertex o, int w, angles h, angles th)
    {
        origin=o; width=w; heading=h; target_heading=th; rend=r;
        int w3 = pow(w,3);
        for (int i=0; i<w3; ++i) {
            // if(i%7==1) {
                tiles.push_back(new Tile(i,r));
            // } else {
                // tiles.push_back(new       Tile(i,r));
            // }
        }
        blocks.push_back(new Block(5, this, r));
        blocks.push_back(new Block(16, this, r));
    };
    inline vertex get_origin() {return origin;};
    inline int get_width() {return width;};
    inline angles get_heading_degs() {return heading;};
    inline angles get_heading_rads()
    {
        return {DegToRad(heading.phi), DegToRad(heading.theta), DegToRad(heading.psi)};
    };
    inline void set_heading_degs(angles h) {heading=h;};
    inline void set_heading_rads(angles h)
    {
        heading={RadToDeg(h.phi), RadToDeg(h.theta), RadToDeg(h.psi)};
    };
    inline bool is_heading_square()
    {
        return (!(int(heading.psi)%90) && !(int(heading.theta)%90) && !(int(heading.phi)%90));
    };
    inline std::vector <Tile*> get_tiles() {return tiles;};
    inline bool get_edit_mode() {return edit_mode;};
    inline void toggle_edit_mode() {edit_mode = !edit_mode;};
    void rotate(direction dir);
    void update();
    vertex coords_to_vertex(coords xyz);
    vertex index_to_vertex(int index);
    void edit_tile(int type, int index);
    Tile* get_tile_at(int index);
    Block* get_block_at(int index);
    void erase_tile_at(int index);
    void draw();
};

#endif
