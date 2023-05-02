#include "basic_types.h"

struct File
{
    void* memory;
    int32 size;
};

typedef File   platform_read_file_function(char16* file_name);
typedef bool32 platform_write_file_function(char16* file_name, File file);

struct Offscreen_buffer
{
    void* memory;
    int32 size;
    int32 width;
    int32 height;
    int32 pixel_size;
};

struct Keyboard_state
{
    bool32 key_w;
    bool32 key_a;
    bool32 key_s;
    bool32 key_d;
};

struct Game_storage
{
    void* memory;
    int32 size;
};

struct Shared_data
{
    Offscreen_buffer offscreen;
    Keyboard_state   keyboard;
    Game_storage     game_storage;

    platform_read_file_function*  platform_read_file;
    platform_write_file_function* platform_write_file;
};

struct Game_state
{
    vec2 player_position;
};
