// std imports
#include <stdlib.h>
#include <math.h>
#include <bitset>
#include <map>
#include <vector>
#include <algorithm>
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
#define FOV (250)
#define NEST_WIDTH (3)
const int NEST_VOLUME = NEST_WIDTH * NEST_WIDTH * NEST_WIDTH;

struct vertex  // rendering 3D graphics
{
  int x;
  int y;
  int z;
};

enum game_state
{
  quit=0,
  play,
  pause,
  menu,
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

bool Collision(int target_ind, std::vector<int> nest[]) {
  // return nest[ind].size() > 0;
  return true;
}

game_state PlayLoop(SDL_Renderer* rend, TTF_Font* font,
  std::vector<int> player_nest[], std::vector<int> target_nest[], int* current_ind) {
    std::map<int,bool> keys;
    const int INPUT_POLL_MAX = 10; // number of frames between repeated inputs
    int input_poll_t = 0; // on key down, set to max, on key up, reset to 0

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
        bool w_key = keys[SDLK_w];
        bool a_key = keys[SDLK_a];
        bool s_key = keys[SDLK_s];
        bool d_key = keys[SDLK_d];
        bool dir_key_pressed = (w_key || a_key || s_key || d_key);
        if (!dir_key_pressed) {
          input_poll_t = 0;
        } else if (input_poll_t) {
          --input_poll_t;
        } else {
          // process input
        }

        // clear the window
        SDL_RenderClear(rend);
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
  std::vector<int> player_nest[], std::vector<int> target_nest[], int* current_ind) {
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

int main(int argc, char *argv[]) {
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

    std::vector<int> player_nest[NEST_VOLUME];
    std::vector<int> target_nest[NEST_VOLUME];
    int current_ind = 0;

    game_state state = game_state::pause;
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
