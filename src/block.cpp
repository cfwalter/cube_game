#include "block.hpp"
#include "tile.hpp"
#include "cube.hpp"


SDL_Surface* Block::front_surface = IMG_Load("../Resources/pink_diamond.png");
SDL_Surface* Block::left_surface  = IMG_Load("../Resources/pink_diamond_left.png");
SDL_Surface* Block::right_surface = IMG_Load("../Resources/pink_diamond_right.png");
SDL_Surface* Block::up_surface    = IMG_Load("../Resources/pink_diamond_up.png");
SDL_Surface* Block::down_surface  = IMG_Load("../Resources/pink_diamond_down.png");

void Block::update()
{
    if (this->move_frame) { this->move_frame += 1; }
    if (this->move_frame == this->move_frames_total) { this->move_frame = 0; }
}


vertex Block::get_current_vertex() {
    vertex current_vert = this->cube->index_to_vertex(this->index);
    vertex old_vert = this->cube->index_to_vertex(this->old_index);
    if (this->move_frame) {
        float dist = 1 - float(this->move_frame) / this->move_frames_total;
        current_vert.x -= (current_vert.x - old_vert.x) * dist;
        current_vert.y -= (current_vert.y - old_vert.y) * dist;
        current_vert.z -= (current_vert.z - old_vert.z) * dist;
    }
    return current_vert;
}


relative_face Block::get_current_face()
{
    if (!this->cube->is_heading_square()) return relative_face::FACE_FRONT;
    vertex unit_v;
    if (this->face == MAX_X) unit_v = { 1,  0,  0};
    if (this->face == MIN_X) unit_v = {-1,  0,  0};
    if (this->face == MAX_Y) unit_v = { 0,  1,  0};
    if (this->face == MIN_Y) unit_v = { 0, -1,  0};
    if (this->face == MAX_Z) unit_v = { 0,  0,  1};
    if (this->face == MIN_Z) unit_v = { 0,  0, -1};
    vertex rot_v = Rotate(unit_v, this->cube->get_heading_rads());

    if (rot_v.x== 1) return relative_face::FACE_RIGHT;
    if (rot_v.x==-1) return relative_face::FACE_LEFT;
    if (rot_v.y== 1) return relative_face::FACE_BOTTOM;
    if (rot_v.y==-1) return relative_face::FACE_TOP;
    if (rot_v.z== 1) return relative_face::FACE_BACK;
    if (rot_v.z==-1) return relative_face::FACE_FRONT;

    return relative_face::FACE_FRONT;
}


void Block::draw()
{
    vertex current_vert = this->get_current_vertex();
    SDL_Texture* img;
    switch (this->get_current_face()) {
        case relative_face::FACE_RIGHT:  img = this->right_texture; break;
        case relative_face::FACE_LEFT:   img = this->left_texture; break;
        case relative_face::FACE_BOTTOM: img = this->down_texture; break;
        case relative_face::FACE_TOP:    img = this->up_texture; break;
        case relative_face::FACE_FRONT:  img = this->front_texture; break;
        case relative_face::FACE_BACK:   img = this->front_texture; break;
    }
    if (current_vert.z <= 0) return;
    int u = FOV*current_vert.x/current_vert.z+CENTER_X;
    int v = FOV*current_vert.y/current_vert.z+CENTER_Y;
    int w, h;
    SDL_QueryTexture(img, NULL, NULL, &w, &h);
    double wz = w/current_vert.z;
    double hz = h/current_vert.z;
    SDL_Rect r = {int(u-wz*0.5), int(v-hz*0.5), int(wz), int(hz)};
    SDL_RenderCopy(this->rend, img, NULL, &r);
}


bool Block::move(direction dir)
{
    if (this->moved_already) {
        return true;
    }
    this->moved_already = true;
    // already_moved is reset after each frame,
    // ensure that it won't be moved more than once per turn

    relative_face rf = this->get_current_face();
    coords next_xyz = this->cube->get_next_coords(rf, dir, this->index);

    const int max = this->cube->get_width()-1;
    bool out_x = next_xyz.x < 0 || next_xyz.x > max;
    bool out_y = next_xyz.y < 0 || next_xyz.y > max;
    bool out_z = next_xyz.z < 0 || next_xyz.z > max;
    if (out_x || out_y || out_z) return false;

    int next_index = XYZ_to_index(next_xyz, this->cube->get_width());

    // 1. Check if tile is walkable
    Tile* next_tile = this->cube->get_tile_at(next_index);
    if (!next_tile->is_walkable(GAMEPLAY_OBJ_TYPE::GOT_BLOCK)) return false;

    // 2. push the blocks in front
    Block* next_block = this->cube->get_block_at(next_index);
    if (next_block && !next_block->move(dir)) {
        return false;
    }
    this->old_index = this->index;
    this->index = next_index;
    this->move_frame = 1;

    // 3. attempt to move linked blocks
    // TODO: change to blockchains
    Block* other_b;
    for (int i=0; i<this->cube->get_blockchains().size(); ++i) {
        other_b = this->cube->get_blockchains().at(i)->get_other_block(this);
        if (other_b) other_b->move(dir);
    }

    return true;
}


void Block::toggle_linked_block(Block* b) {

    for (int i=0; i<this->cube->get_blockchains().size(); ++i) {
        if (this->cube->get_blockchains().at(i)->get_other_block(this) == b) {
            this->cube->get_blockchains().erase(this->cube->get_blockchains().begin()+i);
            return;
        }
    }
    this->cube->add_blockchain(new BlockChain(this, b, this->rend));
};


void BlockChain::draw()
{
    vertex a_vert = this->block_a->get_current_vertex();
    vertex b_vert = this->block_b->get_current_vertex();
    if (a_vert.z <=0 || b_vert.z <=0) return;
    const double dd = 0.04;
    const double dx = b_vert.x - a_vert.x;
    const double dy = b_vert.y - a_vert.y;
    const double dz = b_vert.z - a_vert.z;
    const double mag = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
    const double norm_x = dx / mag;
    const double norm_y = dy / mag;
    const double norm_z = dz / mag;
    const double osx = norm_x * dd * double(this->offset_n) / max_offset_n;
    const double osy = norm_y * dd * double(this->offset_n) / max_offset_n;
    const double osz = norm_z * dd * double(this->offset_n) / max_offset_n;
    const int n = mag / dd;
    SDL_Point points[n];
    for (int i=0; i<n; ++i) {
        const double z = a_vert.z + norm_z * dd * i + osz;
        int u = FOV * (a_vert.x + norm_x * dd * i + osx) / z + CENTER_X;
        int v = FOV * (a_vert.y + norm_y * dd * i + osy) / z + CENTER_Y;
        points[i] = {u, v};
    }
    SDL_RenderDrawPoints(this->rend, points, n);
}
