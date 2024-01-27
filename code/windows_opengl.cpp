#include "windows_opengl.h"

function void ClearOffscreenBuffer(float32 red, float32 green, float32 blue)
{
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

function void RenderQuad(vec2 position, vec2 size)
{
    glBegin(GL_QUADS);

    glVertex2f(position.x - size.x / 2.0f, position.y - size.y / 2.0f);
    glVertex2f(position.x - size.x / 2.0f, position.y + size.y / 2.0f);
    glVertex2f(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
    glVertex2f(position.x + size.x / 2.0f, position.y - size.y / 2.0f);

    glEnd();
}

function void RenderCircle(vec2 position, float32 radius)
{
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(position.x, position.y);

    int32 circle_precision = 32;
    for (int32 i = 0; i <= circle_precision; i += 1)
    {
        float32 angle = (2 * PI32) * (i / (float32)circle_precision);
        glVertex2f(position.x + radius * cos(angle), position.y + radius * sin(angle));
    }

    glEnd();
}

function void RenderBitmap(vec2 position, Bitmap bitmap)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bitmap.width, bitmap.height,
                 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (void*)bitmap.memory);

    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glVertex2f(position.x - bitmap.width / 2.0f, position.y - bitmap.height / 2.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(position.x - bitmap.width / 2.0f, position.y + bitmap.height / 2.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(position.x + bitmap.width / 2.0f, position.y + bitmap.height / 2.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(position.x + bitmap.width / 2.0f, position.y - bitmap.height / 2.0f);
    glTexCoord2f(0.0f, 1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

function void ProcessRenderCommands(Render_commands_buffer* render_commands)
{
    void* current_command = render_commands->memory.begin;
    for(int32 index = 0; index < render_commands->entry_count; index += 1)
    {
        Render_command_entry_header* entry_header = (Render_command_entry_header*)current_command;
        current_command = (char8*)current_command + sizeof(Render_command_entry_header);
        switch(entry_header->command_type)
        {
            case RENDER_BITMAP_COMMAND:
            {
                Render_bitmap_command* render_bitmap_command = (Render_bitmap_command*)current_command;
                Bitmap bitmap = render_bitmap_command->bitmap;
                vec2 position = render_bitmap_command->position;

                RenderBitmap(position, bitmap);

                current_command = (char8*)current_command + sizeof(Render_bitmap_command);
                break;
            }
            case SET_CAMERA_COMMAND:
            {
                Set_camera_command* set_camera_command = (Set_camera_command*)current_command;
                vec2 position = set_camera_command->position;
                vec2 dimension = set_camera_command->dimension;

                float32 orthographic_projection[16] = { 2.0f / dimension.width, 0.0f, 0.0f, 0.0f,
                                                        0.0f, -2.0f / dimension.height, 0.0f, 0.0f,
                                                        0.0f, 0.0f, 1.0f, 0.0f,
                                                        -position.x / dimension.width * 2.0f, position.y / dimension.height * 2.0f, 0.0f, 1.0f };
                glLoadMatrixf(orthographic_projection);

                current_command = (char8*)current_command + sizeof(Set_camera_command);
                break;
            }
        }
    }
}
