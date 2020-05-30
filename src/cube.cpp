#include "cube.hpp"
#include "selector.hpp"
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

/* Cube file format:
width
selector-index phi theta psi
tile0.index tile0.type tile1.index tile1.type ...
block0.index block0.face block1.index block1.face...
blockchain0.a.index blockchain0.b.index
blockchain1.a.index blockchain1.b.index
.
.
.
*/

void Cube::save_to_disk(int home_ind)
{
    // std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // char file_name[43];
    // std::strftime(file_name, sizeof(file_name), "%F_%T.txt", std::localtime(&t));
    printf("saving...\n");
    std::ofstream ofile;
    ofile.open("out.txt", std::fstream::out);
    // width
    ofile << this->width << std::endl;
    // home & heading
    ofile << home_ind << ' ' <<  this->heading.phi << ' ' << this->heading.theta << ' ' << this->heading.psi << ' ' << std::endl;
    // tiles
    for (int i=0; i<this->tiles.size(); ++i) {
        ofile << this->tiles.at(i)->get_index() << ' ' << this->tiles.at(i)->get_type() << ' ';
    }
    ofile << std::endl;
    // blocks
    for (int i=0; i<this->blocks.size(); ++i) {
        ofile << this->blocks.at(i)->get_index() << ' ' << this->blocks.at(i)->get_face() << ' ';
    }
    ofile << std::endl;
    // blockchains
    for (int i=0; i<this->blockchains.size(); ++i) {
        ofile << this->blockchains.at(i)->get_block_a()->get_index() << ' ';
        ofile << this->blockchains.at(i)->get_block_b()->get_index() << std::endl;
    }
    ofile.close();
    printf("done.\n\n");
};

void Cube::load_from_disk(int lvl, Selector * select)
{
    int i=0;
    std::string file_name;
    for(auto& p: std::filesystem::directory_iterator("../Resources/levels")) {
        if (lvl==std::stoi(std::string(p.path().filename()).substr(0,2))) {
            file_name = p.path();
        }
    }
    if (!file_name.length()) return;
    std::ifstream ifile(file_name);
    std::string line;
    int temp_type, temp_index;

    // width
    std::getline(ifile, line);
    this->width = std::stoi(line, NULL);

    // home & heading
    std::getline(ifile, line);
    std::istringstream iss(line);
    iss >> temp_index >> this->heading.phi >> this->heading.theta >> this->heading.psi;
    this->target_heading = this->heading;
    select->set_index(temp_index);
    select->reset();

    // tiles
    std::getline(ifile, line);
    iss.str(line);
    this->tiles.clear();
    while (iss >> temp_index >> temp_type) {
        Tile* tile;
        switch(temp_type) {
            case TT_OPEN_TILE: tile = new OpenTile(temp_index, this, this->rend); break;
            case TT_WALL_TILE: tile = new WallTile(temp_index, this, this->rend); break;
            case TT_POINTER_ONLY_TILE: tile = new PointerOnlyTile(temp_index, this, this->rend); break;
            case TT_BLOCK_ONLY_TILE: tile = new BlockOnlyTile(temp_index, this, this->rend); break;
            case TT_FINISH_TILE: tile = new FinishTile(temp_index, this, this->rend); break;
            case TT_EMPTY_TILE: tile = new EmptyTile(temp_index, this, this->rend); break;
            default: return;
        }
        this->tiles.push_back(tile);
    }
    this->sort_tiles();

    // blocks
    std::getline(ifile, line);
    iss.str(line);
    iss.clear();
    this->blocks.clear();
    while (iss >> temp_index >> temp_type) {
        this->blocks.push_back(new Block(temp_index, CUBE_FACE(temp_type), this, this->rend));
    }

    // blockchains
    this->blockchains.clear();
    while(std::getline(ifile, line)) {
        iss.str(line);
        iss.clear();
        iss >> temp_index; Block* block_a = this->get_block_at(temp_index);
        iss >> temp_index; Block* block_b = this->get_block_at(temp_index);
        block_a->toggle_linked_block(block_b);
    }
    ifile.close();
}


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

void Cube::erase_block_at(int index)
{
    for (int i=0; i<this->blocks.size(); ++i) {
        if (this->blocks.at(i)->get_index() == index) {
            Block * b = this->blocks.at(i);
            for (int i=0; i<this->blockchains.size(); ++i) {
                if (this->blockchains.at(i)->get_other_block(b)) {
                    this->blockchains.erase(this->blockchains.begin()+i);
                    return;
                }
            }
            this->blocks.erase(this->blocks.begin() + i);
            return;
        }
    }
}

void Cube::edit_tile(TILE_TYPE type, int index)
{
    Tile* tile;
    switch(type) {
        case TT_OPEN_TILE: tile = new OpenTile(index, this, this->rend); break;
        case TT_WALL_TILE: tile = new WallTile(index, this, this->rend); break;
        case TT_POINTER_ONLY_TILE: tile = new PointerOnlyTile(index, this, this->rend); break;
        case TT_BLOCK_ONLY_TILE: tile = new BlockOnlyTile(index, this, this->rend); break;
        case TT_FINISH_TILE: tile = new FinishTile(index, this, this->rend); break;
        case TT_EMPTY_TILE: tile = new EmptyTile(index, this, this->rend); break;
        default: return;
    }
    this->erase_tile_at(index);
    this->tiles.push_back(tile);
}

void Cube::toggle_block(int index)
{
    Block* b = this->get_block_at(index);
    if (!b) {
        const angles a = this->get_heading_rads();
        vertex rot_x = Rotate({1,0,0}, a);
        vertex rot_y = Rotate({0,1,0}, a);
        vertex rot_z = Rotate({0,0,1}, a);
        CUBE_FACE face;
        if (rot_x.z== 1) {face = CUBE_FACE::MIN_X;}
        if (rot_x.z==-1) {face = CUBE_FACE::MAX_X;}
        if (rot_y.z== 1) {face = CUBE_FACE::MIN_Y;}
        if (rot_y.z==-1) {face = CUBE_FACE::MAX_Y;}
        if (rot_z.z== 1) {face = CUBE_FACE::MIN_Z;}
        if (rot_z.z==-1) {face = CUBE_FACE::MAX_Z;}

        this->blocks.push_back(new Block(index, face, this, this->rend));
        return;
    }
    this->erase_block_at(index);
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

void Cube::update(const Block* held_block)
{
    bool heading_moved = false;
    // TODO: make this pythonic idk
    if (heading.psi   < target_heading.psi)   {heading.psi   += D_ANGLE; heading_moved=true;}
    if (heading.psi   > target_heading.psi)   {heading.psi   -= D_ANGLE; heading_moved=true;}
    if (heading.phi   < target_heading.phi)   {heading.phi   += D_ANGLE; heading_moved=true;}
    if (heading.phi   > target_heading.phi)   {heading.phi   -= D_ANGLE; heading_moved=true;}
    if (heading.theta < target_heading.theta) {heading.theta += D_ANGLE; heading_moved=true;}
    if (heading.theta > target_heading.theta) {heading.theta -= D_ANGLE; heading_moved=true;}

    if (heading_moved) this->sort_tiles();

    for (int i=0; i<this->blocks.size(); ++i) {
        this->blocks.at(i)->update();
        this->blocks.at(i)->reset_moved_already();
    }
    for (int i=0; i<this->blockchains.size(); ++i) {
        this->blockchains.at(i)->update(held_block);
    }
}

void Cube::sort_tiles()
{
    std::sort(this->tiles.begin(), this->tiles.end(),
        [] (Tile * tile_a, Tile * tile_b) -> bool {
            Cube * c = tile_a->get_cube();
            double az = c->index_to_vertex(tile_a->get_index()).z;
            double bz = c->index_to_vertex(tile_b->get_index()).z;
            return az > bz;
        });
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
        block->draw();
    }

    for (int i=0; i<this->blockchains.size(); ++i) {
        this->blockchains.at(i)->draw();
    }
}

bool Cube::is_win()
{
    for (int i=0; i<this->tiles.size(); ++i) {
        if (!this->tiles.at(i)->is_win()) return false;
    }
    return true;
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


coords Cube::get_next_coords(relative_face rf, direction dir, int current_i)
{
    const angles a = this->get_heading_rads();
    vertex rot_x = Rotate({1,0,0}, a);
    vertex rot_y = Rotate({0,1,0}, a);
    vertex rot_z = Rotate({0,0,1}, a);
    axis u_axis, v_axis; // u=left/right, v=up/down
    int lft_u, rgt_u, top_v, bot_v;
    const int max = this->width-1;
    int modifier_u = 1; int modifier_v = 1;
    axis a1, a2;
    switch (rf) {
        case relative_face::FACE_LEFT:   modifier_u = -1;
        case relative_face::FACE_RIGHT:  a1=axis::AXIS_Z; a2=axis::AXIS_Y; modifier_v = 0; break;
        case relative_face::FACE_TOP:    modifier_v = -1;
        case relative_face::FACE_BOTTOM: a1=axis::AXIS_X; a2=axis::AXIS_Z; modifier_u = 0; break;
        case relative_face::FACE_BACK:   modifier_u = -1; modifier_v = -1;
        case relative_face::FACE_FRONT:  a1=axis::AXIS_X; a2=axis::AXIS_Y; break;
    }

    double x1 = get_axis(rot_x, a1); double x2 = get_axis(rot_x, a2);
    double y1 = get_axis(rot_y, a1); double y2 = get_axis(rot_y, a2);
    double z1 = get_axis(rot_z, a1); double z2 = get_axis(rot_z, a2);

    if (x1== 1) {u_axis=axis::AXIS_X; lft_u=0;   rgt_u=max;}
    if (x1==-1) {u_axis=axis::AXIS_X; lft_u=max; rgt_u=0;}
    if (y1== 1) {u_axis=axis::AXIS_Y; lft_u=0;   rgt_u=max;}
    if (y1==-1) {u_axis=axis::AXIS_Y; lft_u=max; rgt_u=0;}
    if (z1== 1) {u_axis=axis::AXIS_Z; lft_u=0;   rgt_u=max;}
    if (z1==-1) {u_axis=axis::AXIS_Z; lft_u=max; rgt_u=0;}

    if (x2== 1) {v_axis=axis::AXIS_X; top_v=0;   bot_v=max;}
    if (x2==-1) {v_axis=axis::AXIS_X; top_v=max; bot_v=0;}
    if (y2== 1) {v_axis=axis::AXIS_Y; top_v=0;   bot_v=max;}
    if (y2==-1) {v_axis=axis::AXIS_Y; top_v=max; bot_v=0;}
    if (z2== 1) {v_axis=axis::AXIS_Z; top_v=0;   bot_v=max;}
    if (z2==-1) {v_axis=axis::AXIS_Z; top_v=max; bot_v=0;}

    int du = (rgt_u - lft_u) / (max) * modifier_u;
    int dv = (bot_v - top_v) / (max) * modifier_v;

    coords curr_xyz = Index_to_XYZ(current_i, this->width);
    coords next_xyz = curr_xyz;  // keep track of original, but alter the new

    switch(dir) {
        case direction::up:    AddAxis(&next_xyz, -dv, v_axis); break;
        case direction::down:  AddAxis(&next_xyz,  dv, v_axis); break;
        case direction::left:  AddAxis(&next_xyz, -du, u_axis); break;
        case direction::right: AddAxis(&next_xyz,  du, u_axis); break;
        case direction::null: break;
    }
    return next_xyz;
}
