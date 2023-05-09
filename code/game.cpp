#if BUILD_DEBUG
#include "game.h"
#endif

#include "math.cpp"
#include "renderer_software.cpp"

#if BUILD_DEBUG
extern "C"
#else
function
#endif
bool32 initialize_game(Shared_data shared_data)
{
    Offscreen_buffer offscreen = shared_data.offscreen;
    Game_state* game = (Game_state*)shared_data.game_storage.memory;

    game->player_position = vec2{50.0f, 50.0f};

    return true;
}

#if BUILD_DEBUG
extern "C"
#else
function
#endif
void update_game(Shared_data shared_data)
{
    Keyboard_state keyboard = shared_data.keyboard;
    Game_state* game = (Game_state*)shared_data.game_storage.memory;

    game->player_position.x += 1.0f * keyboard.key_d - 1.0f * keyboard.key_a;
    game->player_position.y += 1.0f * keyboard.key_w - 1.0f * keyboard.key_s;
}

#if BUILD_DEBUG
extern "C"
#else
function
#endif
void render_game(Shared_data shared_data)
{
    Offscreen_buffer offscreen = shared_data.offscreen;
    Game_state* game = (Game_state*)shared_data.game_storage.memory;

    render_gradient(offscreen);
    render_rectangle(offscreen, game->player_position, vec2{ 10.0f, 10.0f });
}
