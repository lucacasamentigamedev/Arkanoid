//to compile game and create a new main.exe use this command on root Arkanoid folder "clang -o main.exe .\src\*.c -Llib -lraylibdll -Iinclude"

#include "game.h"
#include "raylib.h"
#include "stdlib.h"

/***********************************************game settings***********************************************/

int quit = 0;
game_state state = TITLE;
const int screen_width = 800;
const int screen_height = 600;
const float gravity = 2.0f;
int frame_count = 0;
int paused = 0;
int saved = 0;
int saved_frame_count = 0;
int load_file_data = 0;
int* savegame = NULL;

/***********************************************gameplay elements***********************************************/

//player
struct player
{
    Vector2 position;
    Vector2 size;
    float speed;
    Rectangle bounds;
    int lives;
} player;

#define LIVES 3

//ball
struct ball
{
    Vector2 position;
    float radius;
    Vector2 speed;
    int active;
} ball;

//bricks
#define BRICK_LINES         5
#define BRICKS_PER_LINE     10
#define BRICK_HEIGHT        40

struct brick 
{
    Vector2 position;
    Vector2 size;
    Rectangle bounds;
    int active;
} bricks[BRICK_LINES][BRICKS_PER_LINE];

//particles
#define PARTICLE_LINES              5
#define PARTICLES_PER_LINE          10
#define NUM_PARTICLE_GENERATORS     3

//single particle
struct particle
{
    Vector2 position;
    Vector2 size;
    Vector2 speed;
    int active;
};

//particles generator
struct particle_generator
{
    struct particle particles[PARTICLE_LINES * PARTICLES_PER_LINE];
    Color color;
    int active;
} particle_generators[NUM_PARTICLE_GENERATORS];

/***********************************************assets***********************************************/

Font font;
Texture2D player_texture;
Texture2D ball_texture;
Texture2D brick_texture;
Sound bounce_sound;
Sound explosion_sound;
Sound start_sound;

/***********************************************init***********************************************/

void game_init()
{
    //init configs
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screen_width, screen_height, "Hello raylib: Breakout");
    SetTargetFPS(60);
    InitAudioDevice();

    //load assets
    font = LoadFont("./resources/setback.png");
    player_texture = LoadTexture("./resources/paddle.png");
    ball_texture = LoadTexture("./resources/ball.png");
    brick_texture = LoadTexture("./resources/brick.png");
    bounce_sound = LoadSound("./resources/bounce.wav");
    explosion_sound = LoadSound("./resources/explosion.wav");
    start_sound = LoadSound("./resources/start.wav");

    //init player
    player.position = (Vector2){ screen_width / 2, screen_height * 7 / 8 };
    player.size = (Vector2){100.0f, 25.0f};
    player.speed = 8.0f;
    player.lives = LIVES;
    player.bounds.x = player.position.x;
    player.bounds.y = player.position.y;
    player.bounds.width = player.size.x;
    player.bounds.height = player.size.y;

    //int ball
    ball.radius = 10.0f;
    ball.position = (Vector2){ player.position.x + player.size.x / 2, player.position.y - ball.radius };
    ball.speed.x = 4.0f;
    ball.speed.y = 4.0f;
    ball.active = 0;

    //init bricks
    for(int i = 0; i < BRICK_LINES; i++)
        for(int j = 0; j < BRICKS_PER_LINE; j++)
        {
            bricks[i][j].size = (Vector2){ screen_width / BRICKS_PER_LINE, BRICK_HEIGHT };
            bricks[i][j].position = (Vector2){ j * bricks[i][j].size.x, i * bricks[i][j].size.y };
            bricks[i][j].bounds.x = bricks[i][j].position.x;
            bricks[i][j].bounds.y = bricks[i][j].position.y;
            bricks[i][j].bounds.width = bricks[i][j].size.x;
            bricks[i][j].bounds.height = bricks[i][j].size.y;
            bricks[i][j].active = 1;
        }
}

/***********************************************state = TITLE***********************************************/

void game_title_process_input(void)
{
    //quit if player press X window
    if(WindowShouldClose())
        quit = 1;

    //go to main game if player press enter
    if(IsKeyPressed(KEY_ENTER))
    {
        PlaySound(start_sound);
        state = PLAY;
    }

    //load game
    if(IsKeyPressed('L'))
    {
        //file exists
        if(FileExists("savegame.bkt"))
        {
            load_file_data = 1;
            int datasize = (BRICK_LINES * BRICKS_PER_LINE + 1) * sizeof(int);
            savegame = (int*)LoadFileData("savegame.bkt", &datasize);
            for(int i = 0; i < BRICK_LINES; i++)
                for(int j = 0; j < BRICKS_PER_LINE; j++)
                {
                    bricks[i][j].active = *savegame;
                    savegame++;
                }
            player.lives = *savegame;
            PlaySound(start_sound);
            state = PLAY;
        }
        //file doesn't exists
        else
        {
            PlaySound(start_sound);
            state = PLAY;
        }
    }
}

void game_title_update(void)
{
    frame_count++;
}

void game_title_draw(void)
{
    BeginDrawing();

    //clear
    ClearBackground(BLACK);
    //draw title
    Vector2 font_position = {160, 100};
    DrawTextEx(font, "Breakout", font_position, 100.0f, 1.0f, RED);
    //draw blink press [enter] to play
    if(frame_count / 60.0f > 2)
    {
        DrawText("press [enter] to play", 100, 300, 50, DARKGREEN);
        DrawText("press [L] to load game", 100, 400, 50, DARKGREEN);
        if(frame_count / 60.0f > 2.5f)
            frame_count = 0;
    }

    EndDrawing();
}

/***********************************************state = PLAY***********************************************/

void game_play_process_input(void)
{
    //quit if player press X windows
    if(WindowShouldClose())
        quit = 1;

    //move player left
    if(IsKeyDown(KEY_LEFT))
        player.position.x -= player.speed;
    //move player right
    if(IsKeyDown(KEY_RIGHT))
        player.position.x += player.speed;

    //move ball
    if(!ball.active && IsKeyPressed(KEY_SPACE))
    {
        ball.active = 1;
        ball.speed.x = 0.0f;
        ball.speed.y = -5.0f;
    }

    //pause game
    if(IsKeyPressed('P'))
        paused = !paused;

    //save game
    if(IsKeyPressed('S'))
    {
        saved = 1;
        saved_frame_count = frame_count;
        int *savedata = (int*)malloc((BRICK_LINES * BRICKS_PER_LINE + 1) * sizeof(int));
		int *p = savedata;
		for (int i = 0; i < BRICK_LINES; i++)
			for (int j = 0; j < BRICKS_PER_LINE; j++)
			{
				*p = bricks[i][j].active;
				p++;
			}

		*p = player.lives;
		SaveFileData("savegame.bkt", savedata, (BRICK_LINES * BRICKS_PER_LINE + 1 ) * sizeof(int));
		free(savedata);
    }
}

void game_play_update(void)
{
    if(paused)
        return;

    //update player position and confine him on screen
    if(player.position.x <= 0)
        player.position.x = 0;
    if(player.position.x + player.size.x >= screen_width)
        player.position.x = screen_width - player.size.x;

    //update player collisions
    player.bounds.x = player.position.x;
    player.bounds.y = player.position.y;

    //ball is active 
    if(ball.active)
    {
        //update ball position
        ball.position.x += ball.speed.x;
        ball.position.y += ball.speed.y;

        //ball collision right & left
        if(ball.position.x - ball.radius <= 0 || ball.position.x + ball.radius >= screen_width)
            ball.speed.x *= -1;

        //ball collision up
        if(ball.position.y - ball.radius <= 0)
            ball.speed.y *= -1;
        
        //ball collision down = reset position and disactive.
        if(ball.position.y >= screen_height)
        {
            ball.active = 0;
            ball.position.y = player.position.y - ball.radius;
            player.lives--;
            //lose condition
            if(player.lives <= 0)
                state = GAME_OVER;
        }
        
        //check collision ball <-> player
        if(CheckCollisionCircleRec(ball.position, ball.radius, player.bounds))
        {
            //bounce up based on collision point
            ball.speed.x = (ball.position.x - (player.position.x + player.size.x / 2)) / player.size.x * 5.0f;
            ball.speed.y *= -1;
            //sound
            PlaySound(bounce_sound);
        }

        //collision ball <-> bricks
        for (int i = 0; i < BRICK_LINES; i++)
            for (int j = 0; j < BRICKS_PER_LINE; j++)
                //if brick is active and collide with ball
                if(bricks[i][j].active && CheckCollisionCircleRec(ball.position, ball.radius, bricks[i][j].bounds))
                {
                    //disactive brick and bounce ball
                    bricks[i][j].active = 0;
                    ball.speed.y *= -1;

                    //sound
                    PlaySound(explosion_sound);

                    //generate particles
                    for (int k = 0; k < NUM_PARTICLE_GENERATORS; k++) {
                        if(!particle_generators[k].active)
                        {
                            particle_generators[k].active = 1;
                            for (int l = 0; l < PARTICLE_LINES; l++)
                                for (int m = 0; m < PARTICLES_PER_LINE; m++)
                                {
                                    particle_generators[k].particles[m + l * PARTICLES_PER_LINE].position.x = bricks[i][j].position.x + m * bricks[i][j].size.x / PARTICLES_PER_LINE;
                                    particle_generators[k].particles[m + l * PARTICLES_PER_LINE].position.y = bricks[i][j].position.y + l * bricks[i][j].size.y / PARTICLE_LINES;
                                    particle_generators[k].particles[m + l * PARTICLES_PER_LINE].size.x = bricks[i][j].size.x / PARTICLES_PER_LINE;
                                    particle_generators[k].particles[m + l * PARTICLES_PER_LINE].size.y = bricks[i][j].size.y / PARTICLE_LINES;
                                    particle_generators[k].particles[m + l * PARTICLES_PER_LINE].speed.x = particle_generators[k].particles[m + l * PARTICLES_PER_LINE].position.x - (bricks[i][j].position.x + bricks[i][j].size.x / 2);
                                    particle_generators[k].particles[m + l * PARTICLES_PER_LINE].speed.y = particle_generators[k].particles[m + l * PARTICLES_PER_LINE].position.y - (bricks[i][j].position.y + bricks[i][j].size.y / 2);
                                    particle_generators[k].particles[m + l * PARTICLES_PER_LINE].active = 1;
                                    particle_generators[k].color = (i + j) % 2 ? GRAY : DARKGRAY;
                                }
                        }
                    }

                    //win condition = no brick with active = 1
                    int win = 1;
                    for (int i = 0; i < BRICK_LINES; i++)
                        for (int j = 0; j < BRICKS_PER_LINE; j++)
                            if(bricks[i][j].active)
                            {
                                win = 0;
                                break;
                            }
                    if(win)
                        state = GAME_OVER;
                }
                    
    }
    else
        //ball is stick to player and follow his position
        ball.position.x = player.position.x + player.size.x / 2.0f;

    //update particles
    for (int i = 0; i < NUM_PARTICLE_GENERATORS; i++)
        if (particle_generators[i].active)
        {
            for (int l = 0; l < PARTICLE_LINES; l++)
				for (int m = 0; m < PARTICLES_PER_LINE; m++)
                    if (particle_generators[i].particles[m + l * PARTICLES_PER_LINE].active)
                    {
                        //gravity
                        particle_generators[i].particles[m + l * PARTICLES_PER_LINE].speed.y += gravity;
                        //normal movement
                        particle_generators[i].particles[m + l * PARTICLES_PER_LINE].position.x += particle_generators[i].particles[m + l * PARTICLES_PER_LINE].speed.x;
                        particle_generators[i].particles[m + l * PARTICLES_PER_LINE].position.y += particle_generators[i].particles[m + l * PARTICLES_PER_LINE].speed.y;
                        //if exit, disactive
                        if (particle_generators[i].particles[m + l * PARTICLES_PER_LINE].position.y >= screen_height)
                            particle_generators[i].particles[m + l * PARTICLES_PER_LINE].active = 0;
                    }
            
            //disactive generator if all particles are out of screen
            particle_generators[i].active = 0;
			for (int l = 0; l < PARTICLE_LINES; l++)
				for (int m = 0; m < PARTICLES_PER_LINE; m++)
					if (particle_generators[i].particles[m + l * PARTICLES_PER_LINE].active)
					{
						particle_generators[i].active = 1; 
						break;
					}
        }

}

void game_play_draw(void)
{
    BeginDrawing();

    //clear
    ClearBackground(BLUE);
    //draw player
    DrawTexture(player_texture, player.position.x, player.position.y, WHITE);
    //draw ball
    DrawTexture(ball_texture, ball.position.x - ball.radius, ball.position.y - ball.radius, WHITE);
    //draw lives
    for (int i = 0; i < player.lives; i++)
        DrawRectangle(20 + 40 * i, screen_height - 35, 30, 30, RED);
    //draw bricks
    for (int i = 0; i < BRICK_LINES; i++)
        for (int j = 0; j < BRICKS_PER_LINE; j++)
            if(bricks[i][j].active)
            {
                if((i+j) % 2)
                    DrawTextureEx(brick_texture, bricks[i][j].position, 0.0f, 2.0f, GRAY);
                else
                    DrawTextureEx(brick_texture, bricks[i][j].position, 0.0f, 2.0f, DARKGRAY);
            }
    // draw particles
    for (int i = 0; i < NUM_PARTICLE_GENERATORS; i++)
        if (particle_generators[i].active)
            for (int l = 0; l < PARTICLE_LINES; l++)
				for (int m = 0; m < PARTICLES_PER_LINE; m++)
                    if (particle_generators[i].particles[m + l * PARTICLES_PER_LINE].active)
                        DrawRectangleV(particle_generators[i].particles[m + l * PARTICLES_PER_LINE].position, particle_generators[i].particles[m + l * PARTICLES_PER_LINE].size, particle_generators[i].color);

    //pause game
    if(paused)
    {
        struct Vector2 pausePosition = { 160.0f, 300.0f };
        DrawTextEx(font, "game paused", pausePosition, 50.0f, 1.0f, DARKGRAY);
    }

    //save game
    if(saved)
    {
        DrawText("game saved", 600, 500, 20, DARKGRAY);
        if((frame_count - saved_frame_count) / 60.0f > 2.0f)
            saved = 0;
    }
    
    EndDrawing();
}

/***********************************************state = GAME OVER***********************************************/

void game_over_process_input(void)
{
    //quit if player press windows X
    if(WindowShouldClose())
        quit = 1;

    //reset and re-enter in game if player press enter
    if(IsKeyPressed(KEY_ENTER))
    {
        //reset frame count
        frame_count = 0;
        //reset bricks
        for (int i = 0; i < BRICK_LINES; i++)
            for (int j = 0; j < BRICKS_PER_LINE; j++)
                bricks[i][j].active = 1;
        //reset lives   
        player.lives = LIVES;
        //reset ball
        ball.position.x = player.position.x + player.size.x / 2.0f;
        ball.position.y = player.position.y - ball.radius;
        ball.active = 0;
        //reset generators
        for(int i = 0; i < NUM_PARTICLE_GENERATORS; i++) {
            particle_generators[i].active = 0;
        }
        //go to title
        state = TITLE;
    }
}

void game_over_update(void)
{
    frame_count++;
}

void game_over_draw(void)
{
    BeginDrawing();

    //clear
    ClearBackground(BLACK);
    //draw title
    Vector2 font_position = {160, 100};
    DrawTextEx(font, "Game Over", font_position, 100.0f, 1.0f, RED);
    //draw blink press [enter] to play
    if(frame_count / 60.0f > 2)
    {
        DrawText("press [enter] to play", 100, 300, 50, DARKGREEN);
        if(frame_count / 60.0f > 2.5f)
            frame_count = 0;
    }

    EndDrawing();
}

/***********************************************deinit***********************************************/

void game_deinit(void)
{
    UnloadFont(font);
    UnloadTexture(player_texture);
    UnloadTexture(ball_texture);
    UnloadTexture(brick_texture);
    UnloadSound(bounce_sound);
    UnloadSound(explosion_sound);
    UnloadSound(start_sound);
    if (load_file_data)
    {
        load_file_data = 0;
        UnloadFileData(savegame);
    }
}