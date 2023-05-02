function void render_rectangle(Offscreen_buffer offscreen, vec2 position, vec2 size)
{
    int32 render_min_x = round_float32_to_int32(position.x);
    int32 render_min_y = round_float32_to_int32(position.y);
    int32 render_max_x = round_float32_to_int32(position.x + size.x);
    int32 render_max_y = round_float32_to_int32(position.y + size.y);

    if (render_min_x < 0)
    {
        render_min_x = 0;
    }
    if (render_min_y < 0)
    {
        render_min_y = 0;
    }
    if (render_max_x > offscreen.width)
    {
        render_max_x = offscreen.width;
    }
    if (render_max_y > offscreen.height)
    {
        render_max_y = offscreen.height;
    }

    int32* offscreen_row = (int32*)offscreen.memory + render_min_y * offscreen.width + render_min_x;
    for (int32 y = render_min_y; y < render_max_y; y += 1)
    {
        int32* offscreen_pixel = offscreen_row;
        for (int32 x = render_min_x; x < render_max_x; x += 1)
        {
            *offscreen_pixel = 0xFF << 16 | 0xFF << 8 | 0xFF << 0;

            offscreen_pixel += 1;
        }
        offscreen_row += offscreen.width;
    }
}

function void render_gradient(Offscreen_buffer offscreen)
{
    int32* offscreen_row = (int32*)offscreen.memory;
    for (int32 y = 0; y < offscreen.height; y += 1)
    {
        int32* offscreen_pixel = offscreen_row;
        for (int32 x = 0; x < offscreen.width; x += 1)
        {
            *offscreen_pixel = (x & y) << 16 | (x ^ y) << 8 | (x | y) << 0;

            offscreen_pixel += 1;
        }
        offscreen_row += offscreen.width;
    }
}
