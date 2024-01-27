#if BUILD_DEBUG
#include "game.h"
#endif

function Memory_region allocate(Memory_region* memory, int32 size)
{
    Memory_region result = {};
    void* next = (char8*)memory->current + size;

    Assert(next < memory->end);

    result.begin = memory->current;
    result.current = result.begin;
    result.end = next;

    memory->current = next;

    return result;
}

#include "renderer.cpp"

function Bitmap load_bitmap(platform_read_file_function* read_file, char16* file_name)
{
    Bitmap result = {};

    File file = read_file(file_name);

    Bitmap_header* bitmap_header = (Bitmap_header*)file.memory;
    result.memory = (void*)((char8*)file.memory + bitmap_header->bitmap_offset);
    result.size = bitmap_header->image_size;
    result.width = bitmap_header->width;
    result.height = bitmap_header->height;

    return result;
}

function void handle_blob(vec2* blob_position, vec2* blob_velocity,
                          bool32 key_left, bool32 key_right, bool32 key_up)
{
    float32 current_gravitation = GRAVITATION;
    if (key_up)
    {
        if (blob_position->y >= GROUND_PLANE_HEIGHT)
        {
            blob_velocity->y = BLOBBY_JUMP_ACCELERATION;
        }
        current_gravitation -= BLOBBY_JUMP_BUFFER;
    }

    blob_velocity->x = (1.0f * key_right - 1.0f * key_left) * BLOBBY_SPEED;

    *blob_position += vec2{ blob_velocity->x, 0.5f * current_gravitation + blob_velocity->y };

    blob_velocity->y += current_gravitation;

    if (blob_position->y > GROUND_PLANE_HEIGHT)
    {
        blob_position->y = GROUND_PLANE_HEIGHT;
        blob_velocity->y = 0.0f;
    }
}

function bool32 circle_to_circle(vec2 first_position,  float32 first_radius,
                                 vec2 second_position, float32 second_radius)
{
    vec2 distance = first_position - second_position;
    float32 radius_sum = first_radius + second_radius;
    return length_square(distance) < radius_sum * radius_sum;
}

function void handle_blob_ball_collision(vec2* ball_position, vec2* ball_velocity,
                                         vec2 blob_position, vec2 blob_velocity)
{
    vec2 collision_center = blob_position;

    // check for impact
    bool32 isCollide = false;
    vec2 lower_sphere_position = vec2{ blob_position.x, blob_position.y + BLOBBY_LOWER_SPHERE };
    vec2 upper_sphere_position = vec2{ blob_position.x, blob_position.y - BLOBBY_UPPER_SPHERE };
    if (circle_to_circle(*ball_position, BALL_RADIUS, lower_sphere_position, BLOBBY_LOWER_RADIUS))
    {
        collision_center.y += BLOBBY_LOWER_SPHERE;
        isCollide = true;
    }
    else if (circle_to_circle(*ball_position, BALL_RADIUS, upper_sphere_position, BLOBBY_UPPER_RADIUS))
    {
        collision_center.y -= BLOBBY_UPPER_SPHERE;
        isCollide = true;
    }

    if (isCollide)
    {
        // ok, if we get here, there actually was a collision

        // calculate hit intensity
        vec2 temp = blob_velocity - *ball_velocity;
        float32 intensity = Min(1.0f, sqrt(temp.x * temp.x + temp.y * temp.y) / 25.0f);

        // set ball velocity
        *ball_velocity = -(collision_center - *ball_position);
        *ball_velocity = normalize(*ball_velocity);
        *ball_velocity = *ball_velocity * BALL_COLLISION_VELOCITY;
        *ball_position += *ball_velocity;
    }
}

#if BUILD_DEBUG
extern "C"
#else
function
#endif
bool32 initialize_game(Shared_data* shared_data)
{
    Offscreen_buffer offscreen = shared_data->offscreen;
    shared_data->game_state = allocate(&shared_data->memory, sizeof(Game_state)).begin;
    shared_data->render_commands.memory = allocate(&shared_data->memory, Megabytes(64));

    Game_state* game = (Game_state*)shared_data->game_state;
    game->player_position = vec2{200.0f, GROUND_PLANE_HEIGHT};
    game->player_second_position = vec2{600.0f, GROUND_PLANE_HEIGHT};
    game->ball_position = vec2{200.0f, STANDARD_BALL_HEIGHT};

    game->background = load_bitmap(shared_data->platform_read_file, L"background.bmp");
    game->ball = load_bitmap(shared_data->platform_read_file, L"ball.bmp");
    game->blobby = load_bitmap(shared_data->platform_read_file, L"blobby.bmp");

    return true;
}

#if BUILD_DEBUG
extern "C"
#else
function
#endif
void update_game(Shared_data* shared_data)
{
    Input input = shared_data->input;
    Game_state* game = (Game_state*)shared_data->game_state;

    game->camera.x = game->camera.x + (1.0f * input.keyboard[KEY_RIGHT] - 1.0f * input.keyboard[KEY_LEFT]) * 10.0f;

    // Handle blob
    handle_blob(&game->player_position, &game->player_velocity, input.keyboard[KEY_A], input.keyboard[KEY_D], input.keyboard[KEY_W]);
    handle_blob(&game->player_second_position, &game->player_second_velocity, input.keyboard[KEY_LEFT], input.keyboard[KEY_RIGHT], input.keyboard[KEY_UP]);

    // Move ball
    game->ball_position += vec2{game->ball_velocity.x, 0.5f * BALL_GRAVITATION + game->ball_velocity.y};
    game->ball_velocity.y += BALL_GRAVITATION;

    // Handle ball and blobby collision
    handle_blob_ball_collision(&game->ball_position, &game->ball_velocity, game->player_position, game->player_velocity);
    handle_blob_ball_collision(&game->ball_position, &game->ball_velocity, game->player_second_position, game->player_second_velocity);

    // Handle ball and world collision
    {
        // Ball to roof collision
        if(game->ball_position.y - BALL_RADIUS < 0)
        {
            game->ball_velocity = vec2{ game->ball_velocity.x, -game->ball_velocity.y };
            game->ball_velocity *= 0.95f;
            game->ball_position.y = BALL_RADIUS;
        }

        // Ball to ground Collision
        if (game->ball_position.y + BALL_RADIUS > GROUND_PLANE_HEIGHT_MAX)
        {
            game->ball_velocity = vec2{ game->ball_velocity.x, -game->ball_velocity.y };
            game->ball_velocity *= 0.95f * 0.6f;
            game->ball_position.y = GROUND_PLANE_HEIGHT_MAX - BALL_RADIUS;
        }

        // Border Collision
        if (game->ball_position.x - BALL_RADIUS <= LEFT_PLANE && game->ball_velocity.x < 0.0)
        {
            game->ball_velocity = vec2{-game->ball_velocity.x, game->ball_velocity.y};
            // set the ball's position
            game->ball_position.x = LEFT_PLANE + BALL_RADIUS;
        }
        else if (game->ball_position.x + BALL_RADIUS >= RIGHT_PLANE && game->ball_velocity.x > 0.0)
        {
            game->ball_velocity = vec2{-game->ball_velocity.x, game->ball_velocity.y};
            // set the ball's position
            game->ball_position.x = RIGHT_PLANE - BALL_RADIUS;
        }
        else if (game->ball_position.y > NET_SPHERE_POSITION && fabs(game->ball_position.x - NET_POSITION_X) < BALL_RADIUS + NET_RADIUS)
        {
            bool32 right = game->ball_position.x - NET_POSITION_X > 0;
            game->ball_velocity = vec2{-game->ball_velocity.x, game->ball_velocity.y};
            // set the ball's position so that it touches the net
            game->ball_position.x = NET_POSITION_X + (right ? (BALL_RADIUS + NET_RADIUS) : (-BALL_RADIUS - NET_RADIUS));
        }
        else
        {
            // Net Collisions
            float32 ball_net_distance = length(vec2{NET_POSITION_X, NET_SPHERE_POSITION} - vec2{game->ball_position});
            if (ball_net_distance < NET_RADIUS + BALL_RADIUS)
            {
                // calculate
                vec2 normal = normalize(vec2{NET_POSITION_X, NET_SPHERE_POSITION} - vec2{game->ball_position});

                // normal component of kinetic energy
                float32 perp_ekin = dot_product(normal, game->ball_velocity);
                perp_ekin *= perp_ekin;
                // parallel component of kinetic energy
                float32 para_ekin = length_square(game->ball_velocity) - perp_ekin;

                // the normal component is damped stronger than the parallel component
                // the values are ~ 0.85 and ca. 0.95, because speed is sqrt(ekin)
                perp_ekin *= 0.7f;
                para_ekin *= 0.9f;

                float32 new_speed = sqrt(perp_ekin + para_ekin);

                vec2 reflect = vec2{game->ball_velocity - (normal * 2 * dot_product(game->ball_velocity, normal))};
                game->ball_velocity = normalize(reflect) * new_speed;

                // pushes the ball out of the net
                game->ball_position = vec2{NET_POSITION_X, NET_SPHERE_POSITION} - normal * (NET_RADIUS + BALL_RADIUS);
            }
        }
    }

    // Collision between blobby and the net
    if (game->player_position.x + BLOBBY_LOWER_RADIUS > NET_POSITION_X - NET_RADIUS)
       game->player_position.x = NET_POSITION_X - NET_RADIUS - BLOBBY_LOWER_RADIUS;

    if (game->player_second_position.x - BLOBBY_LOWER_RADIUS < NET_POSITION_X + NET_RADIUS)
        game->player_second_position.x = NET_POSITION_X + NET_RADIUS + BLOBBY_LOWER_RADIUS;

    // Collision between blobby and the border
    if (game->player_position.x < LEFT_PLANE)
        game->player_position.x = LEFT_PLANE;

    if (game->player_second_position.x > RIGHT_PLANE)
        game->player_second_position.x = RIGHT_PLANE;
}

#if BUILD_DEBUG
extern "C"
#else
function
#endif
void render_game(Shared_data* shared_data)
{
    Offscreen_buffer offscreen = shared_data->offscreen;
    Game_state* game = (Game_state*)shared_data->game_state;
    Render_commands_buffer* render_commands = &shared_data->render_commands;

    reset_render_command_buffer(render_commands);

    game->camera = game->player_position;
    //push_set_camera_command(render_commands, game->camera, {800.0f, 600.0f});

    push_render_bitmap_command(render_commands, {800.0f / 2.0f, 600.0f / 2.0f}, game->background);
    push_render_bitmap_command(render_commands, game->player_position, game->blobby);
    push_render_bitmap_command(render_commands, game->player_second_position, game->blobby);
    push_render_bitmap_command(render_commands, game->ball_position, game->ball);
}
