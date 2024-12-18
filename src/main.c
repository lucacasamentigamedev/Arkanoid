#include "game.h"

int main(int argc, char **argv)
{
    game_init();

    //game loop
    while(!quit)
    {
        switch (state)
        {
        case TITLE:
            game_title_process_input();
            game_title_update();
            game_title_draw();
            break;

        case PLAY:
            game_play_process_input();
            game_play_update();
            game_play_draw();
            break;

        case GAME_OVER:
            game_over_process_input();
            game_over_update();
            game_over_draw();
            break;
        
        default:
            break;
        }
    }

    game_deinit();

    return 0;
}