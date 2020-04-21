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
#define NEST_WIDTH (9)
const int NW_2 = NEST_WIDTH * NEST_WIDTH;
const int NW_3 = NEST_WIDTH * NEST_WIDTH * NEST_WIDTH;

struct vertex  // rendering 3D graphics
{
  float x;
  float y;
  float z;
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
  float r;
  float i;
  float j;
  float k;
};

void DrawWire(SDL_Renderer * renderer, float x1, float y1, float z1, float x2, float y2, float z2)
{
  if (z1 <= 0 || z2 <= 0) {
    return;
  }
  int center_x = WINDOW_WIDTH / 2;
  int center_y = WINDOW_HEIGHT / 2;
  SDL_RenderDrawLine(renderer,
    FOV*x1/z1+center_x, FOV*y1/z1+center_y,
    FOV*x2/z2+center_x, FOV*y2/z2+center_y);
}

void DrawWire(SDL_Renderer * renderer, vertex p1, vertex p2)
{
  DrawWire(renderer, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
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

quat ToQuaternion(float yaw, float pitch, float roll) // yaw (Z), pitch (Y), roll (X)
{
    // Abbreviations for the various angular functions
    float cy = cos(yaw * 0.5);
    float sy = sin(yaw * 0.5);
    float cp = cos(pitch * 0.5);
    float sp = sin(pitch * 0.5);
    float cr = cos(roll * 0.5);
    float sr = sin(roll * 0.5);

    quat q;
    q.r = cr * cp * cy + sr * sp * sy;
    q.i = sr * cp * cy - cr * sp * sy;
    q.j = cr * sp * cy + sr * cp * sy;
    q.k = cr * cp * sy - sr * sp * cy;

    return q;
}

void DrawCube(SDL_Renderer * renderer, float ox, float oy, float oz, float pitch, float yaw, float roll, int red, int grn, int blu)
{
  int center_x = WINDOW_WIDTH / 2;
  int center_y = WINDOW_HEIGHT / 2;
  int u, v;
  float x,y,z, inv_mag_p, mag_p, p_diff;
  quat p;
  quat q = ToQuaternion(pitch, yaw, roll);
  quat inv_q = {q.r, -q.i, -q.j, -q.k};
  const float w = 1;
  float space = w/NEST_WIDTH; // 1/nw * w
  float half_width = w/2;
  SDL_Point points[NW_3];
  SDL_Point out_points[NW_3*4]; // fading points
  for (int i=0; i<NW_3; ++i) {
    x = (i % NEST_WIDTH) * space - half_width;
    y = ((i / NEST_WIDTH)%NEST_WIDTH) * space - half_width;
    z = (i / (NW_2)) * space - half_width;
    p_diff = pow(x,2)+pow(y,2)+pow(z,2);
    mag_p = sqrt(p_diff);
    inv_mag_p = 1/mag_p;
    p = {0, x*inv_mag_p, y*inv_mag_p, z*inv_mag_p};
    // printf("%f %f %f\n", x, y, z);
    p = QuatMul(QuatMul(q, p), inv_q); // pqp^-1
    x=p.i*mag_p+ox;
    y=p.j*mag_p+oy;
    z=p.k*mag_p+oz;
    // if (x < ox-half_width || x>ox+half_width ||
    //   y < oy-half_width || y>oy+half_width ||
    //   z < oz-half_width || z>oz+half_width) {

    // }
    if (z>0) {
      u = FOV*x/z+center_x;
      v = FOV*y/z+center_y;
      points[i] = {u, v};
      for (int j=0; j<4; ++j){
        out_points[4*i+j] = {u+(j%2)?-1:1, u+(j/2)?-1:1};
      }
      points[i] = {int(FOV*x/z+center_x), int(FOV*y/z+center_y)};
    } else {
      points[i] = {-1, -1}; // off screen
      for (int j=0; j<4; ++j){
        out_points[4*i+j] = {-1, -1};
      }
      printf("%f %f %f\n\n", x, y, z);
    }
  }
  SDL_SetRenderDrawColor(renderer, red, grn, blu, 255);
  SDL_RenderDrawPoints(renderer, points, NW_3);

  // TODO: experiment with alpha channel
  SDL_SetRenderDrawColor(renderer, 4*red/5, 4*grn/5, 4*blu/5, 255);
  SDL_RenderDrawPoints(renderer, points, NW_3*4);
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

    float a = 0.0;
    float b = 1.0;
    float c = 0.0;
    float pitch = 0.0;
    float yaw = 0.0;
    float roll = 0.0;

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
        bool left = keys[SDLK_LEFT];
        bool right = keys[SDLK_RIGHT];
        bool down = keys[SDLK_DOWN];
        bool up__key = keys[SDLK_w] || keys[SDLK_UP];
        bool rgt_key = keys[SDLK_a] || keys[SDLK_RIGHT];
        bool dwn_key = keys[SDLK_s] || keys[SDLK_DOWN];
        bool lft_key = keys[SDLK_d] || keys[SDLK_LEFT];
        bool dir_key_pressed = (up__key || rgt_key || dwn_key || lft_key);
          // input_poll_t = INPUT_POLL_MAX;
        if (up__key) {
          roll -= 0.2;
        }
        if (dwn_key) {
          roll += 0.2;
        }
        if (lft_key) {
          yaw += 0.2;
        }
        if (rgt_key) {
          yaw -= 0.2;
        }
        // clear the window
        SDL_RenderClear(rend);

        DrawCube(rend, -1, 0, 3, pitch, yaw, roll, 0, 255, 0);
        DrawCube(rend, 1, 0, 3, pitch, yaw, roll, 255, 0, 255);

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
