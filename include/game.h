//dichiarazioni

#ifndef GAME_H
#define GAME_H

typedef enum game_state
{
    TITLE, PLAY, GAME_OVER
} game_state;

extern int quit;
extern game_state state;

void game_init();

//title screen
void game_title_process_input(void);
void game_title_update(void);
void game_title_draw(void);

//game screen
void game_play_process_input(void);
void game_play_update(void);
void game_play_draw(void);

//game over screen
void game_over_process_input(void);
void game_over_update(void);
void game_over_draw(void);

void game_deinit(void);

#endif // GAME_H