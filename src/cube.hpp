#ifndef CUBE_HPP
#define CUBE_HPP
#include "main.hpp"
#include "tile.hpp"
#include "block.hpp"
#include "quat.hpp"
#include "common.hpp"

class Selector;

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
    std::vector <BlockChain*> blockchains;
public:
    inline Cube(SDL_Renderer * r, vertex o, int w, angles h, angles th)
    {
        origin=o; width=w; heading=h; target_heading=th; rend=r;
        int w3 = pow(w,3);
        for (int i=0; i<w3; ++i) {
            tiles.push_back(new OpenTile(i,this,r));
        }
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
    inline std::vector <BlockChain*> * get_blockchains() {return &blockchains;};
    inline void add_blockchain(BlockChain* bc) {blockchains.push_back(bc);};
    inline bool get_edit_mode() {return edit_mode;};
    inline void toggle_edit_mode() {edit_mode = !edit_mode;};
    void rotate(direction dir);
    void update(const Block* held_block);
    vertex coords_to_vertex(coords xyz);
    vertex index_to_vertex(int index);
    void edit_tile(TILE_TYPE type, int index);
    void add_tile(TILE_TYPE type, int index);
    Tile* get_tile_at(int index);
    Block* get_block_at(int index);
    void erase_tile_at(int index);
    void toggle_block(int index);
    void erase_block_at(int index);
    void save_to_disk(int home_ind);
    void load_from_disk(int lvl, Selector* select);
    void sort_tiles();
    coords get_next_coords(relative_face rf, direction dir, int current_i);
    bool is_win();
    void draw();
};

#endif
