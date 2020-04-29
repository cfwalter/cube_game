#include "tile.hpp"

SDL_Surface* Tile::img_surface = IMG_Load("Resources/blu_ring.png");


void Tile::draw(double x, double y, double z)
{
    int u = FOV*x/z+CENTER_X;
    int v = FOV*y/z+CENTER_Y;
    // int w, h;
    // SDL_QueryTexture(img_texture, NULL, NULL, &w, &h);
    double wz = w/z;
    double hz = h/z;
    SDL_Rect r = {int(u-wz*0.5), int(v-hz*0.5), int(wz), int(hz)};
    SDL_RenderCopy(this->rend, img_texture, NULL, &r);
}

SDL_Surface* RedTile::img_surface = IMG_Load("Resources/red_ring.png");


SDL_Surface* YellowTile::img_surface = IMG_Load("Resources/yellow_ring.png");
SDL_Surface* YellowTile::red_img_surface = IMG_Load("Resources/red_ring.png");
