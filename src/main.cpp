// local imports
#include "main.hpp"
#include "constants.hpp"
#include "quat.hpp"
#include "cube.hpp"
#include "selector.hpp"

enum game_state
{
    quit=0,
    play,
    pause,
    menu,
};

bool Collision(int target_ind, std::vector<int> nest[])
{
    // return nest[ind].size() > 0;
    return true;
}

game_state PlayLoop(SDL_Renderer* rend, TTF_Font* font,
    std::vector<int> player_nest[], std::vector<int> target_nest[], int* current_ind)
{
    const int WIDTH = 5;
    Cube play_cube = Cube(rend, {0, 0, 2}, WIDTH, {0,0,0}, {0,0,0});

    Selector select = Selector(rend, 0, &play_cube);
    LinkedBlockEditor editor = LinkedBlockEditor(rend, &play_cube);

    bool quit_state = false;
    bool pause_state = false;

    direction rotate_cube = direction::null;

    SDL_Surface* edit_msg_surface = TTF_RenderText_Solid(font, "EDIT", {255, 255, 255});
    SDL_Texture* edit_msg_texture = SDL_CreateTextureFromSurface(rend, edit_msg_surface);
    SDL_Rect edit_msg_rect = {20, 20, edit_msg_surface->w, edit_msg_surface->h};
    SDL_FreeSurface(edit_msg_surface);

    SDL_Surface* win_msg_surface = TTF_RenderText_Solid(font, "WIN!", {255, 255, 255});
    SDL_Texture* win_msg_texture = SDL_CreateTextureFromSurface(rend, win_msg_surface);
    SDL_Rect win_msg_rect = {WINDOW_WIDTH/2 - win_msg_surface->w/2, 20,
        win_msg_surface->w, win_msg_surface->h};
    SDL_FreeSurface(win_msg_surface);

    for(std::map<int,bool> keys; !(quit_state || pause_state); ) {
        // process events
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type)
            {
                case SDL_QUIT: quit_state = true; break;
                case SDL_KEYDOWN: if (ev.key.repeat == 0) {keys[ev.key.keysym.sym] = true;} break;
                case SDL_KEYUP: keys[ev.key.keysym.sym] = false; break;
            }
        }

        // process input
        pause_state = keys[SDLK_p];
        quit_state = keys[SDLK_ESCAPE] || quit_state;
        bool up__key = keys[SDLK_w] || keys[SDLK_UP];
        bool rgt_key = keys[SDLK_a] || keys[SDLK_RIGHT];
        bool dwn_key = keys[SDLK_s] || keys[SDLK_DOWN];
        bool lft_key = keys[SDLK_d] || keys[SDLK_LEFT];
        bool dir_key_pressed = (up__key || rgt_key || dwn_key || lft_key);

        if (keys[SDLK_e]) {
            play_cube.toggle_edit_mode();
            editor.clear_selection();
            keys[SDLK_e] = false;
        }

        if (play_cube.get_edit_mode()) {
            if (keys[SDLK_1]) {
                play_cube.edit_tile(TILE_TYPE::TT_OPEN_TILE, select.get_index());
                keys[SDLK_1]=false;
            }
            if (keys[SDLK_2]) {
                play_cube.edit_tile(TILE_TYPE::TT_WALL_TILE, select.get_index());
                keys[SDLK_2]=false;
            }
            if (keys[SDLK_3]) {
                play_cube.edit_tile(TILE_TYPE::TT_POINTER_ONLY_TILE, select.get_index());
                keys[SDLK_3]=false;
            }
            if (keys[SDLK_4]) {
                play_cube.edit_tile(TILE_TYPE::TT_BLOCK_ONLY_TILE, select.get_index());
                keys[SDLK_4]=false;
            }
            if (keys[SDLK_5]) {
                play_cube.edit_tile(TILE_TYPE::TT_FINISH_TILE, select.get_index());
                keys[SDLK_5]=false;
            }
            if (keys[SDLK_0]) {
                play_cube.edit_tile(TILE_TYPE::TT_EMPTY_TILE, select.get_index());
                keys[SDLK_0]=false;
            }
            if (keys[SDLK_b]) {
                play_cube.toggle_block(select.get_index());
                keys[SDLK_b]=false;
            }
        }

        if (play_cube.is_heading_square() && dir_key_pressed) {
            direction dir;
            if (up__key) dir = direction::up;
            if (rgt_key) dir = direction::right;
            if (dwn_key) dir = direction::down;
            if (lft_key) dir = direction::left;
            // TODO: refactor this so select.move calls cube.rotate(dir);
            rotate_cube = select.move(dir);
        }
        if (keys[SDLK_x]) {
            if (play_cube.get_edit_mode()) {
                editor.toggle_selection(play_cube.get_block_at(select.get_index()));
            } else {
                select.toggle_holding();
            }
            keys[SDLK_x] = false;
        }

        if (play_cube.is_heading_square() && rotate_cube != direction::null) {
            play_cube.rotate(rotate_cube);
            rotate_cube = direction::null;
        }

        play_cube.update(select.get_held_block());
        select.update();

        // clear the window
        SDL_RenderClear(rend);

        SDL_SetRenderDrawColor(rend, 200, 200, 200, 255);
        play_cube.draw();
        select.draw();
        editor.draw();
        if (play_cube.get_edit_mode()) {
            SDL_RenderCopy(rend, edit_msg_texture, NULL, &edit_msg_rect);
        }

        if (play_cube.is_win()) {
            SDL_RenderCopy(rend, win_msg_texture, NULL, &win_msg_rect);
        }

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

    SDL_Window* win = SDL_CreateWindow("CUBE",
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
    TTF_Font* font = TTF_OpenFont("../Resources/Westminster.ttf", 24);
    if(!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    std::vector<int> player_nest[0];
    std::vector<int> target_nest[0];
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
