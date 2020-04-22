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

struct angles
{
  double phi;
  double theta;
  double psi;
};

enum game_state
{
  quit=0,
  play,
  pause,
  menu,
};

struct quat
{
  double r;
  double i;
  double j;
  double k;
};

float Rad(int deg)
{
  return (deg*PI/180.0);
}

quat QuatMul(quat p, quat q)
{
  // (ae-bf-cg-dh)+(af+be+ch-dg){i} +(ag-bh+ce+df){j} +(ah+bg-cf+de){k}
  float r = p.r*q.r - p.i*q.i - p.j*q.j - p.k*q.k;
  float i = p.r*q.i + p.i*q.r + p.j*q.k - p.k*q.j;
  float j = p.r*q.j - p.i*q.k + p.j*q.r + p.k*q.i;
  float k = p.r*q.k + p.i*q.j - p.j*q.i + p.k*q.r;
  return {r, i, j, k};
}

void DrawCube(SDL_Renderer * renderer, vertex o, angles a, int red, int grn, int blu)
{
  int center_x = WINDOW_WIDTH / 2;
  int center_y = WINDOW_HEIGHT / 2;
  float x,y,z, inv_mag_p, mag_p, p_diff;
  quat p;

  // todo: combine these into one quat
  quat z_q = {cos(a.psi*0.5), 0, 0, sin(a.psi*0.5)}; // unit vector at z=1
  quat inv_z_q = {z_q.r, -z_q.i, -z_q.j, -z_q.k};

  quat y_q = {cos(a.theta*0.5), 0, sin(a.theta*0.5), 0}; // unit vector at y=1
  quat inv_y_q = {y_q.r, -y_q.i, -y_q.j, -y_q.k};

  quat x_q = {cos(a.phi*0.5), sin(a.phi*0.5), 0, 0}; // unit vector at x=1
  quat inv_x_q = {x_q.r, -x_q.i, -x_q.j, -x_q.k};
  const double w = 1.0;
  double space = w/(NEST_WIDTH-1); // 1/nw * w
  double half_width = w/2;
  SDL_Point points[NW_3];
  SDL_Point out_points[NW_3*4]; // fading points
  for (int i=0; i<NW_3; ++i) {
    int ind_x = (i % NEST_WIDTH);
    int ind_y = ((i / NEST_WIDTH)%NEST_WIDTH);
    int ind_z = (i / (NW_2));
    bool front_face = !(ind_x && ind_y && ind_z);
    bool back_face  = !((ind_x+1)%NEST_WIDTH && (ind_y+1)%NEST_WIDTH && (ind_z+1)%NEST_WIDTH);
    if ( front_face || back_face){ //test if
      x = ind_x * space - half_width;
      y = ind_y * space - half_width;
      z = ind_z * space - half_width;
      p = {0, x, y, z};
      p = QuatMul(QuatMul(z_q, p), inv_z_q); // pqp^-1
      p = QuatMul(QuatMul(y_q, p), inv_y_q); // pqp^-1
      p = QuatMul(QuatMul(x_q, p), inv_x_q); // pqp^-1
      x=p.i+o.x; y=p.j+o.y; z=p.k+o.z;

      if (z>0) {
        // TODO: clean this up, possibly replace with asset
        double d = 0.005;
        out_points[4*i+0] = {int((FOV*(x+d))/z+center_x), int((FOV*(y+0))/z+center_y)};
        out_points[4*i+1] = {int((FOV*(x-d))/z+center_x), int((FOV*(y+0))/z+center_y)};
        out_points[4*i+2] = {int((FOV*(x+0))/z+center_x), int((FOV*(y+d))/z+center_y)};
        out_points[4*i+3] = {int((FOV*(x+0))/z+center_x), int((FOV*(y-d))/z+center_y)};
        points[i] = {int(FOV*x/z+center_x), int(FOV*y/z+center_y)};
      } else {
        points[i] = {-1, -1}; // off screen
        for (int j=0; j<4; ++j){
          out_points[4*i+j] = {-1, -1};
        }
        printf("%f %f %f\n\n", x, y, z);
      }
    }
  }
  SDL_SetRenderDrawColor(renderer, red, grn, blu, 255);
  SDL_RenderDrawPoints(renderer, points, NW_3);

  // TODO: experiment with alpha channel
  // SDL_SetRenderDrawColor(renderer, 2*red/5, 2*grn/5, 2*blu/5, 255);
  // SDL_RenderDrawPoints(renderer, out_points, NW_3*4);
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

    bool quit_state = false;
    bool pause_state = false;
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
        input_poll_t = INPUT_POLL_MAX;
        if (!(abs(psi)%90) && !(abs(theta)%90) && !(abs(phi)%90)) {
          // Logic here to make sure cube always rotates relative to the camera
          // TODO: fix edge case, possibly replace with logic table
          if (up__key != dwn_key) {
            int d = 90 * (up__key ? 1 : -1);
            int deg = (theta%360+360)%360;
            switch (deg/90){
              case 0: target_phi -= d; break;
              case 1: target_psi -= d; break;
              case 2: target_phi -= d; break;
              case 3: target_psi += d; break;
            }
          }
          if (lft_key != rgt_key) {
            int d = 90 * (rgt_key ? 1 : -1);
            int deg = (phi%360+360)%360;
            switch (deg/90){
              case 0: target_theta -= d; break;
              case 1: target_psi += d; break;
              case 2: target_theta += d; break;
              case 3: target_psi -= d; break;
            }
          }
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
        DrawCube(rend, {-1, 0, 3}, a, 0, 255, 0);
        DrawCube(rend, {1, 0, 3}, a, 255, 0, 255);

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
