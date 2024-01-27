function void reset_render_command_buffer(Render_commands_buffer* buffer)
{
    buffer->memory.current = buffer->memory.begin;
    buffer->entry_count = 0;
}

function void push_render_bitmap_command(Render_commands_buffer* buffer,
                                         vec2 position, Bitmap bitmap)
{
    int32 command_header_size = sizeof(Render_command_entry_header);
    int32 command_size = sizeof(Render_bitmap_command);

    Memory_region command_header_memory = allocate(&buffer->memory, command_header_size);
    Memory_region command_memory = allocate(&buffer->memory, command_size);

    Render_command_entry_header* command_header = (Render_command_entry_header*)command_header_memory.begin;
    command_header->command_type = RENDER_BITMAP_COMMAND;

    Render_bitmap_command* render_bitmap_command = (Render_bitmap_command*)command_memory.begin;
    render_bitmap_command->position = position;
    render_bitmap_command->bitmap = bitmap;

    buffer->entry_count += 1;
}

function void push_set_camera_command(Render_commands_buffer* buffer,
                                      vec2 position, vec2 dimension)
{
    int32 command_header_size = sizeof(Render_command_entry_header);
    int32 command_size = sizeof(Set_camera_command);

    Memory_region command_header_memory = allocate(&buffer->memory, command_header_size);
    Memory_region command_memory = allocate(&buffer->memory, command_size);

    Render_command_entry_header* command_header = (Render_command_entry_header*)command_header_memory.begin;
    command_header->command_type = SET_CAMERA_COMMAND;

    Set_camera_command* set_camera_command = (Set_camera_command*)command_memory.begin;
    set_camera_command->position = position;
    set_camera_command->dimension = dimension;

    buffer->entry_count += 1;
}
