#include "basic_types.h"
#include "camera.h"

struct File
{
    void* memory;
    int32 size;
};

struct Memory_region
{
    void* begin;
    void* current;
    void* end;
};

struct Bitmap
{
    void* memory;
    uint32 size;
    int32 width;
    int32 height;
};

#include "renderer.h"

#pragma pack(push, 1)
struct Bitmap_header
{
    // Bitmap file header
    uint16 type;
    uint32 file_size;
    uint16 reserved[2];
    uint32 bitmap_offset;

    // Bitmap info header
    uint32 info_size;
    int32 width;
    int32 height;
    uint16 planes;
    uint16 bit_count;
    uint32 compression;
    uint32 image_size;
    int32 horizontal_resolution;
    int32 vertical_resolutiion;
    uint32 color_number;
    uint32 important_color_number;
};
#pragma pack(pop)

typedef int32  platform_get_file_size_function(char16* file_name);
typedef File   platform_read_file_function(char16* file_name);
typedef bool32 platform_write_file_function(char16* file_name, File file);

typedef void platform_clear_offscreen_buffer(float32 red, float32 green, float32 blue);

struct Offscreen_buffer
{
    void* memory; // unused
    int32 size; // unused
    int32 width;
    int32 height;
    int32 pixel_size; // unused
};

enum Key_code_id
{
    KEY_EMPTY,
    KEY_MOUSE_LEFT, KEY_MOUSE_MIDDLE, KEY_MOUSE_RIGHT,
    KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
    KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_SPACE, KEY_BACKSPACE, KEY_ENTER, KEY_SHIFT, KEY_TAB,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

    KEY_COUNT = 255,
};

struct Input
{
    bool32 keyboard[KEY_COUNT];
};

struct Shared_data
{
    Offscreen_buffer offscreen;
    Input            input;
    Memory_region    memory;

    void* game_state;
    Render_commands_buffer render_commands;

    platform_get_file_size_function* platform_get_file_size_function;
    platform_read_file_function*     platform_read_file;
    platform_write_file_function*    platform_write_file;
};

struct Game_state
{
    vec2 player_position;
    vec2 player_velocity;
    vec2 player_second_position;
    vec2 player_second_velocity;

    vec2 ball_position;
    vec2 ball_velocity;

    Bitmap background;
    Bitmap blobby;
    Bitmap ball;

    vec2 camera;
};

// Border Settings
global float32 LEFT_PLANE = 0.0f;
global float32 RIGHT_PLANE = 800.0f;

// Blobby settings
global float32 BLOBBY_HEIGHT = 89.0f;
global float32 BLOBBY_UPPER_SPHERE = 19.0f;
global float32 BLOBBY_UPPER_RADIUS = 25.0f;
global float32 BLOBBY_LOWER_SPHERE = 13.0f;
global float32 BLOBBY_LOWER_RADIUS = 33.0f;

// Ground Settings
global float32 GROUND_PLANE_HEIGHT_MAX = 500.0f;
global float32 GROUND_PLANE_HEIGHT = GROUND_PLANE_HEIGHT_MAX - BLOBBY_HEIGHT / 2.0f;

// This is exactly the half of the gravitation, I checked it in the original code
global float32 BLOBBY_MAX_JUMP_HEIGHT = GROUND_PLANE_HEIGHT - 206.375f;
global float32 BLOBBY_JUMP_ACCELERATION = -15.1f;

// these values are calculated from the other two
global float32 GRAVITATION = BLOBBY_JUMP_ACCELERATION * BLOBBY_JUMP_ACCELERATION / BLOBBY_MAX_JUMP_HEIGHT;
global float32 BLOBBY_JUMP_BUFFER = GRAVITATION / 2;

// Ball Settings
global float32 BALL_RADIUS = 31.5f;
global float32 BALL_GRAVITATION = 0.287f;
global float32 BALL_COLLISION_VELOCITY = sqrt(0.75f * RIGHT_PLANE * BALL_GRAVITATION);

// Volley Ball Net
global float32 NET_POSITION_X = RIGHT_PLANE / 2;
global float32 NET_POSITION_Y = 438;
global float32 NET_RADIUS = 7;
global float32 NET_SPHERE = 154; // what is the meaning of this value ???????
global float32 NET_SPHERE_POSITION = 284;

global float32 STANDARD_BALL_HEIGHT = 269 + BALL_RADIUS;

// BLOBBY_SPEED is necessary to determine the size of the input buffer
global float32 BLOBBY_SPEED = 4.5f;
global float32 STANDARD_BALL_ANGULAR_VELOCITY = 0.1f;
