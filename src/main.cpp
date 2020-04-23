// std imports
#include <stdlib.h>
#include <math.h>
#include <bitset>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>

// SDL imports
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// local imports
#include "quat.hpp"

#define WINDOW_WIDTH (640)
#define WINDOW_HEIGHT (480)
#define PI (3.14159)
#define FPS (30)
#define Z (12)  // depth from camera to render
#define FOV (450)
#define NEST_WIDTH (5)
const int NW_2 = NEST_WIDTH * NEST_WIDTH;
const int NW_3 = NEST_WIDTH * NEST_WIDTH * NEST_WIDTH;

struct vertex  // rendering 3D graphics
{
  double x;
  double y;
  double z;
};

struct coords // track point on 3D grid
{
  int x;
  int y;
  int z;
};

struct angles
{
  double phi;
  double theta;
  double psi;
};

enum axis
{
  x=0,
  y,
  z,
  none,
};

enum direction
{
  up=0,
  right,
  down,
  left,
  null,
};

enum game_state
{
  quit=0,
  play,
  pause,
  menu,
};

float Rad(int deg)
{
  return (deg*PI/180.0);
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

coords Index_to_XYZ(int index, int width)
{
    return {(index % width), ((index / width)%width), (index / (width*width))};
}

int XYZ_to_index(coords xyz, int width)
{
    return xyz.z*width*width + xyz.y*width + xyz.x;
}

void DrawSelect(SDL_Renderer * renderer, vertex o, angles a, int i, SDL_Texture* img)
{
  // TODO: remove duplicated logic with DrawCube
  int center_x = WINDOW_WIDTH / 2;
  int center_y = WINDOW_HEIGHT / 2;
  float x,y,z;

  const double width = 1.0;
  double space = width/(NEST_WIDTH-1); // 1/nw * width
  double half_width = width/2;
  coords xyz = Index_to_XYZ(i, NEST_WIDTH);
  x = xyz.x * space - half_width;
  y = xyz.y * space - half_width;
  z = xyz.z * space - half_width;
  vertex vert = Rotate({x, y, z}, a);
  x=vert.x+o.x; y=vert.y+o.y; z=vert.z+o.z;

  int u = FOV*x/z+center_x; int v = FOV*y/z+center_y;
  int w, h;
  SDL_QueryTexture(img, NULL, NULL, &w, &h);
  SDL_Rect r = {int(u-w/z*0.5), int(v-w/z*0.5), int(w/z), int(h/z)};
  SDL_RenderCopy(renderer, img, NULL, &r);
}

void DrawCube(SDL_Renderer * renderer, vertex o, angles a, SDL_Texture* img)
{
  int center_x = WINDOW_WIDTH / 2;
  int center_y = WINDOW_HEIGHT / 2;
  float x,y,z;

  const double w = 1.0;
  double space = w/(NEST_WIDTH-1); // 1/nw * w
  double half_width = w/2;
  for (int i=0; i<NW_3; ++i) {
    coords xyz = Index_to_XYZ(i, NEST_WIDTH);
    // TODO: gotta be a better way to do this logic
    bool front_face = !(xyz.x && xyz.y && xyz.z);
    bool back_face  = !((xyz.x+1)%NEST_WIDTH && (xyz.y+1)%NEST_WIDTH && (xyz.z+1)%NEST_WIDTH);
    if ( front_face || back_face){ //test if
      x = xyz.x * space - half_width;
      y = xyz.y * space - half_width;
      z = xyz.z * space - half_width;
      vertex v = Rotate({x, y, z}, a);
      x=v.x+o.x; y=v.y+o.y; z=v.z+o.z;

      if (z>0) {
        int u = FOV*x/z+center_x; int v = FOV*y/z+center_y;
        int w, h;
        SDL_QueryTexture(img, NULL, NULL, &w, &h);
        w = w/z; h = h/z;
        SDL_Rect r = {int(u-w*0.5), int(v-w*0.5), int(w), int(h)};
        SDL_RenderCopy(renderer, img, NULL, &r);
      }
    }
  }
}

void AddAxis(coords* xyz, int n, axis a)
{
  if (a==axis::x) xyz->x += n;
  if (a==axis::y) xyz->y += n;
  if (a==axis::z) xyz->z += n;
}

direction MoveTarget(int* index, angles a, direction dir)
{
  // get rotations of x,y,z with current angle, to see which way is up/down and left/right
  vertex rot_x = Rotate({1,0,0}, a);
  vertex rot_y = Rotate({0,1,0}, a);
  vertex rot_z = Rotate({0,0,1}, a);
  axis u_axis, v_axis; // u=left/right, v=up/down
  int lft_u, rgt_u, top_v, bot_v;
  const int max = NEST_WIDTH-1;

  if (rot_x.x== 1) {u_axis=axis::x; lft_u=0;   rgt_u=max;}
  if (rot_x.x==-1) {u_axis=axis::x; lft_u=max; rgt_u=0;}
  if (rot_y.x== 1) {u_axis=axis::y; lft_u=0;   rgt_u=max;}
  if (rot_y.x==-1) {u_axis=axis::y; lft_u=max; rgt_u=0;}
  if (rot_z.x== 1) {u_axis=axis::z; lft_u=0;   rgt_u=max;}
  if (rot_z.x==-1) {u_axis=axis::z; lft_u=max; rgt_u=0;}

  if (rot_x.y== 1) {v_axis=axis::x; top_v=0;   bot_v=max;}
  if (rot_x.y==-1) {v_axis=axis::x; top_v=max; bot_v=0;}
  if (rot_y.y== 1) {v_axis=axis::y; top_v=0;   bot_v=max;}
  if (rot_y.y==-1) {v_axis=axis::y; top_v=max; bot_v=0;}
  if (rot_z.y== 1) {v_axis=axis::z; top_v=0;   bot_v=max;}
  if (rot_z.y==-1) {v_axis=axis::z; top_v=max; bot_v=0;}

  int du = (rgt_u - lft_u) / (max);
  int dv = (bot_v - top_v) / (max);

  coords curr_xyz = Index_to_XYZ(*index, NEST_WIDTH);
  switch(dir) {
    case direction::up:    AddAxis(&curr_xyz, -dv, v_axis); break;
    case direction::down:  AddAxis(&curr_xyz,  dv, v_axis); break;
    case direction::left:  AddAxis(&curr_xyz, -du, u_axis); break;
    case direction::right: AddAxis(&curr_xyz,  du, u_axis); break;
    case direction::null: break;
  }
  bool out_x = curr_xyz.x < 0 || curr_xyz.x > max;
  bool out_y = curr_xyz.y < 0 || curr_xyz.y > max;
  bool out_z = curr_xyz.z < 0 || curr_xyz.z > max;
  if (out_x || out_y || out_z) {
    return dir;
  }
  *index = XYZ_to_index(curr_xyz, NEST_WIDTH);
  return direction::null;
}

bool Collision(int target_ind, std::vector<int> nest[])
{
  // return nest[ind].size() > 0;
  return true;
}

game_state PlayLoop(SDL_Renderer* rend, TTF_Font* font,
  std::vector<int> player_nest[], std::vector<int> target_nest[], int* current_ind)
{
    std::map<int,bool> keys;
    const int INPUT_POLL_MAX = 5; // number of frames between repeated inputs
    int input_poll_t = 0; // on key down, set to max, on key up, reset to 0
    int input_blocked = false;

    int phi = 0; // phi, theta and psi are in degrees
    int theta = 0;
    int psi = 0;
    int target_phi = 0;
    int target_theta = 0;
    int target_psi = 0;
    const int d_angle = 10;

    int select_index = 0;

    bool quit_state = false;
    bool pause_state = false;
    bool dir_key_dirty = false;

    direction rotate_cube = direction::null;

    SDL_Surface* blu_ring = IMG_Load("Resources/blu_ring.png");
    SDL_Texture* blu_ring_tex = SDL_CreateTextureFromSurface(rend, blu_ring);
    SDL_FreeSurface(blu_ring);

    SDL_Surface* red_ring = IMG_Load("Resources/red_ring.png");
    SDL_Texture* red_ring_tex = SDL_CreateTextureFromSurface(rend, red_ring);
    SDL_FreeSurface(red_ring);

    SDL_Surface* box = IMG_Load("Resources/open_box.png");
    SDL_Texture* box_tex = SDL_CreateTextureFromSurface(rend, box);
    SDL_FreeSurface(box);

    for(std::map<int,bool> keys; !(quit_state || pause_state); ) {
        // process events
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type)
            {
                case SDL_QUIT: quit_state = true; break;
                case SDL_KEYDOWN: keys[ev.key.keysym.sym] = true; break;
                case SDL_KEYUP: keys[ev.key.keysym.sym] = false; break;
            }
        }

        // process input
        pause_state = keys[SDLK_ESCAPE] || keys[SDLK_p];
        bool up__key = keys[SDLK_w] || keys[SDLK_UP];
        bool rgt_key = keys[SDLK_a] || keys[SDLK_RIGHT];
        bool dwn_key = keys[SDLK_s] || keys[SDLK_DOWN];
        bool lft_key = keys[SDLK_d] || keys[SDLK_LEFT];
        bool dir_key_pressed = (up__key || rgt_key || dwn_key || lft_key);
        if (!(up__key || rgt_key || dwn_key || lft_key)) {
          dir_key_dirty = false;
        }

        input_poll_t = INPUT_POLL_MAX;

        if (!(abs(psi)%90) && !(abs(theta)%90) && !(abs(phi)%90) && dir_key_pressed && !dir_key_dirty) {
          direction dir;
          if (up__key) dir = direction::up;
          if (rgt_key) dir = direction::right;
          if (dwn_key) dir = direction::down;
          if (lft_key) dir = direction::left;
          rotate_cube = MoveTarget(&select_index, {Rad(phi), Rad(theta), Rad(psi)}, dir);
          dir_key_dirty = true;
        }

        if (!(abs(psi)%90) && !(abs(theta)%90) && !(abs(phi)%90) && rotate_cube != direction::null) {
          // Logic here to make sure cube always rotates relative to the camera
          // TODO: fix edge case, possibly replace with logic table
          if (rotate_cube == direction::up || rotate_cube == direction::down) {
            int d = 90 * (rotate_cube == direction::down ? 1 : -1);
            int deg = (theta%360+360)%360;
            switch (deg/90){
              case 0: target_phi -= d; break;
              case 1: target_psi -= d; break;
              case 2: target_phi -= d; break;
              case 3: target_psi += d; break;
            }
          }
          if (rotate_cube == direction::left || rotate_cube == direction::right) {
            int d = 90 * (rotate_cube == direction::left ? 1 : -1);
            int deg = (phi%360+360)%360;
            switch (deg/90){
              case 0: target_theta -= d; break;
              case 1: target_psi += d; break;
              case 2: target_theta += d; break;
              case 3: target_psi -= d; break;
            }
          }
          rotate_cube = direction::null;
        }

        // TODO: make this pythonic idk
        if (psi   < target_psi)   psi   += d_angle;
        if (psi   > target_psi)   psi   -= d_angle;
        if (phi   < target_phi)   phi   += d_angle;
        if (phi   > target_phi)   phi   -= d_angle;
        if (theta < target_theta) theta += d_angle;
        if (theta > target_theta) theta -= d_angle;

        // clear the window
        SDL_RenderClear(rend);

        angles a = {Rad(phi), Rad(theta), Rad(psi)};
        // DrawCube(rend, {-1, 0, 3}, a, blu_ring_tex);
        DrawCube(rend, {0, 0, 2}, a, blu_ring_tex);
        DrawSelect(rend, {0, 0, 2}, a, select_index, box_tex);
        // DrawCube(rend, {1, 0, 3}, a, red_ring_tex);

        // debugging output
        // std::string op = "pitch:" + std::to_string(phi%360) +
        //                "  yaw:" + std::to_string(theta%360) +
        //                "  roll:" + std::to_string(psi%360);
        // std::string up__op = (up__key ? "up ":" ");
        // std::string dwn_op = (dwn_key ? "down ":" ");
        // std::string lft_op = (lft_key ? "left ":" ");
        // std::string rgt_op = (rgt_key ? "right":" ");
        // std::string op = up__op + dwn_op + lft_op + rgt_op;
        // SDL_Surface* message = TTF_RenderText_Solid(font, op.c_str(), {255, 255, 255});
        // SDL_Texture* message_tex = SDL_CreateTextureFromSurface(rend, message);
        // SDL_Rect message_rect = {5, 5, message->w, message->h};
        // SDL_RenderCopy(rend, message_tex, NULL, &message_rect);

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderPresent(rend);

        // wait 1/60th of a second
        SDL_Delay(1000/FPS);
    }
    if (quit_state) return game_state::quit;
    if (pause_state) return game_state::pause;
    return game_state::quit;
}

game_state PauseLoop(SDL_Renderer* rend, TTF_Font* font,
  std::vector<int> player_nest[], std::vector<int> target_nest[], int* current_ind)
{
    std::map<int,bool> keys;

    SDL_Surface* message = TTF_RenderText_Solid(font, "PAUSED", {255, 255, 255});
    SDL_Texture* message_tex = SDL_CreateTextureFromSurface(rend, message);
    int paused_y = WINDOW_HEIGHT/2-69;
    SDL_Rect message_rect = {WINDOW_WIDTH/2-message->w/2, paused_y, message->w, message->h};

    SDL_Surface* play_option = TTF_RenderText_Solid(font, "PLAY", {255, 255, 255});
    SDL_Texture* play_option_tex = SDL_CreateTextureFromSurface(rend, play_option);
    int menu_y = WINDOW_HEIGHT/2;
    SDL_Rect play_option_rect = {WINDOW_WIDTH/2-play_option->w/2, menu_y, play_option->w, play_option->h};

    SDL_Surface* quit_option = TTF_RenderText_Solid(font, "QUIT", {255, 255, 255});
    SDL_Texture* quit_option_tex = SDL_CreateTextureFromSurface(rend, quit_option);
    SDL_Rect quit_option_rect = {WINDOW_WIDTH/2-quit_option->w/2, menu_y+30, quit_option->w, quit_option->h};

    SDL_Surface* picker = TTF_RenderText_Solid(font, "*          *", {255, 255, 255});
    SDL_Texture* picker_tex = SDL_CreateTextureFromSurface(rend, picker);
    SDL_Rect picker_rect = {WINDOW_WIDTH/2-picker->w/2, menu_y, picker->w, picker->h};

    bool quit_state = false;
    bool play_state = false;

    int option_picked = 0;
    bool arrow_key_dirty = true;

    // animation loop
    for(std::map<int,bool> keys; !(quit_state || play_state); ) {
        // process events
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type)
            {
                case SDL_QUIT: quit_state = true; break;
                case SDL_KEYDOWN: keys[ev.key.keysym.sym] = true; break;
                case SDL_KEYUP: keys[ev.key.keysym.sym] = false; break;
            }
        }

        play_state = keys[SDLK_ESCAPE];

        if (keys[SDLK_RETURN]) {
          switch (option_picked) {
            case 0: play_state = true; break;
            case 1: quit_state = true; break;
          }
        }

        if (!(keys[SDLK_DOWN] || keys[SDLK_UP])) arrow_key_dirty = true;
        if ((keys[SDLK_DOWN] || keys[SDLK_UP]) && arrow_key_dirty) {
          option_picked = (option_picked + 1) % 2;
          arrow_key_dirty = false;
        }

        // clear the window
        SDL_RenderClear(rend);
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderPresent(rend);

        // wait 1/60th of a second
        SDL_Delay(1000/FPS);
    }

    SDL_FreeSurface(message);
    SDL_DestroyTexture(message_tex);
    SDL_FreeSurface(play_option);
    SDL_DestroyTexture(play_option_tex);
    SDL_FreeSurface(quit_option);
    SDL_DestroyTexture(quit_option_tex);
    SDL_FreeSurface(picker);
    SDL_DestroyTexture(picker_tex);
    if (quit_state) return game_state::quit;
    if (play_state) return game_state::play;
    return game_state::quit;
}

int main(int argc, char *argv[])
{
    // attempt to initialize graphics and timer system
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Untangle",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       WINDOW_WIDTH, WINDOW_HEIGHT,0);
    if (!win) {
        printf("error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // create a renderer, which sets up the graphics hardware
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);
    if (!rend) {
        printf("error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    TTF_Init();
    TTF_Font* font = TTF_OpenFont("Resources/Westminster.ttf", 24);
    if(!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    std::vector<int> player_nest[NW_3];
    std::vector<int> target_nest[NW_3];
    int current_ind = 0;

    game_state state = game_state::play;
    while(state!=game_state::quit) {
      switch(state) {
        case game_state::play:
          state = PlayLoop(rend, font, player_nest, target_nest, &current_ind);
          break;
        case game_state::pause:
          state = PauseLoop(rend, font, player_nest, target_nest, &current_ind);
          break;
        case game_state::quit: break;
        default: break;
      }
    }
    // clean up resources before exiting
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    Mix_Quit();
    SDL_Quit();
    return 0;
}
