#ifndef CUBE_HPP
#define CUBE_HPP
#include "tile.hpp"
#include "block.hpp"
#include "quat.hpp"
#include "common.hpp"

coords Index_to_XYZ(int index, int width)
{
    return {(index % width), ((index / width)%width), (index / (width*width))};
}

int XYZ_to_index(coords xyz, int width)
{
    return xyz.z*width*width + xyz.y*width + xyz.x;
}

vertex Rotate(vertex v, angles a) {
    // todo: combine these into one Quat
    Quat z_q = {cos(a.psi*0.5), 0, 0, sin(a.psi*0.5)}; // unit vector at z=1
    Quat inv_z_q = {z_q.r, -z_q.i, -z_q.j, -z_q.k};

    Quat y_q = {cos(a.theta*0.5), 0, sin(a.theta*0.5), 0}; // unit vector at y=1
    Quat inv_y_q = {y_q.r, -y_q.i, -y_q.j, -y_q.k};

    Quat x_q = {cos(a.phi*0.5), sin(a.phi*0.5), 0, 0}; // unit vector at x=1
    Quat inv_x_q = {x_q.r, -x_q.i, -x_q.j, -x_q.k};

    Quat p = {0, v.x, v.y, v.z};
    p = z_q * p * inv_z_q;
    p = y_q * p * inv_y_q;
    p = x_q * p * inv_x_q;
    return {p.i, p.j, p.k};
}

void AddAxis(coords* xyz, int n, axis a)
{
    if (a==axis::x) xyz->x += n;
    if (a==axis::y) xyz->y += n;
    if (a==axis::z) xyz->z += n;
}

float DegToRad(int deg)
{
    return (deg*PI/180.0);
}

float RadToDeg(float rad)
{
    return (rad/PI*180);
}

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
        blocks.push_back(new Block(5,r));
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

Tile* Cube::get_tile_at(int index)
{
    if (this->tiles.at(index)->get_index() == index) {
        // check that spot directly, will work for loaded levels probably, save you a loop
        return this->tiles.at(index);
    }
    for (int i=0; i<this->tiles.size(); ++i) {
        if (this->tiles.at(i)->get_index() == index) {
            return this->tiles.at(i);
        }
    }
    return NULL;
}

Block* Cube::get_block_at(int index)
{
    for (int i=0; i<this->blocks.size(); ++i) {
        if (this->blocks.at(i)->get_index() == index) {
            return this->blocks.at(i);
        }
    }
    return NULL;
}

void Cube::erase_tile_at(int index)
{
    for (int i=0; i<this->tiles.size(); ++i) {
        if (this->tiles.at(i)->get_index() == index) {
            this->tiles.erase(this->tiles.begin() + i);
            return;
        }
    }
}

void Cube::edit_tile(int type, int index)
{
    Tile* tile;
    switch(type) {
        case 1: tile = new    Tile(index, this->rend); break;
        case 2: tile = new RedTile(index, this->rend); break;
        default: return;
    }
    this->erase_tile_at(index);
    this->tiles.push_back(tile);
}

void Cube::rotate(direction dir)
{
    // Logic here to make sure cube always rotates relative to the camera
    // TODO: fix edge case, possibly replace with logic table
    if (dir == direction::up || dir == direction::down) {
        int d = 90 * (dir == direction::down ? 1 : -1);
        int deg = (int(heading.theta)%360+360)%360;
        switch (deg/90){
            case 0: target_heading.phi -= d; break;
            case 1: target_heading.psi -= d; break;
            case 2: target_heading.phi -= d; break;
            case 3: target_heading.psi += d; break;
        }
    }
    if (dir == direction::left || dir == direction::right) {
        int d = 90 * (dir == direction::left ? 1 : -1);
        int deg = (int(heading.phi)%360+360)%360;
        switch (deg/90){
            case 0: target_heading.theta -= d; break;
            case 1: target_heading.psi += d; break;
            case 2: target_heading.theta += d; break;
            case 3: target_heading.psi -= d; break;
        }
    }
}

void Cube::update()
{
    // TODO: make this pythonic idk
    if (heading.psi   < target_heading.psi)   heading.psi   += D_ANGLE;
    if (heading.psi   > target_heading.psi)   heading.psi   -= D_ANGLE;
    if (heading.phi   < target_heading.phi)   heading.phi   += D_ANGLE;
    if (heading.phi   > target_heading.phi)   heading.phi   -= D_ANGLE;
    if (heading.theta < target_heading.theta) heading.theta += D_ANGLE;
    if (heading.theta > target_heading.theta) heading.theta -= D_ANGLE;

    for (int i=0; i<this->blocks.size(); ++i) {
        this->blocks.at(i)->update();
    }
}

void Cube::draw()
{
    int index;
    Tile* tile;
    for (int i=0; i<this->tiles.size(); ++i) {
        tile = this->tiles.at(i);
        int index = tile->get_index();
        coords xyz = Index_to_XYZ(index, this->width);
        // TODO: gotta be a better way to do this logic
        // TODO: move the front/back face logic to the Cube init
        bool front_face = !(xyz.x && xyz.y && xyz.z);
        bool back_face  = !((xyz.x+1)%width && (xyz.y+1)%width && (xyz.z+1)%width);
        if ( front_face || back_face){
            vertex vert = coords_to_vertex(xyz);
            if (vert.z>0) {
                tile->draw(vert.x, vert.y, vert.z);
            }
        }
    }
    Block* block;
    for (int i=0; i<this->blocks.size(); ++i) {
        block = this->blocks.at(i);
        vertex current_vert = index_to_vertex(block->get_index());
        vertex old_vert = index_to_vertex(block->get_old_index());
        block->draw(current_vert, old_vert);
    }
}

vertex Cube::coords_to_vertex(coords xyz)
{
    vertex o = this->origin;
    angles a = this->get_heading_rads();
    const double w = 1.0;
    double space = w/(this->width-1); // 1/nw * w
    double half_width = w/2;
    double x = xyz.x * space - half_width;
    double y = xyz.y * space - half_width;
    double z = xyz.z * space - half_width;
    vertex v = Rotate({x, y, z}, a);
    v.x += o.x; v.y += o.y; v.z += o.z;
    return v;
}

vertex Cube::index_to_vertex(int index)
{
    return this->coords_to_vertex(Index_to_XYZ(index, width));
}

#endif
