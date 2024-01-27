enum Render_command_type
{
    RENDER_EMPTY_COMMAND,
    RENDER_BITMAP_COMMAND,
    SET_CAMERA_COMMAND,

    RENDER_COMMAND_TYPE_COUNT
};

struct Render_command_entry_header
{
    Render_command_type command_type;
};

struct Render_commands_buffer
{
    Memory_region memory;
    int32 entry_count;
};

struct Render_bitmap_command
{
    vec2 position;
    Bitmap bitmap;
};

struct Set_camera_command
{
    vec2 position;
    vec2 dimension;
};
