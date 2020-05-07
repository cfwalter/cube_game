#include "tile.hpp"

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

SDL_Surface* Tile::img_surface = IMG_Load("../Resources/none.png");
SDL_Surface* OpenTile::img_surface = IMG_Load("../Resources/open_tile.png");
SDL_Surface* WallTile::img_surface = IMG_Load("../Resources/wall_tile.png");
SDL_Surface* PointerOnlyTile::img_surface = IMG_Load("../Resources/pointer_only_tile.png");
SDL_Surface* BlockOnlyTile::img_surface = IMG_Load("../Resources/block_only_tile.png");
