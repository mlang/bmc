/**
 * \file player.c
 * \brief Midi player functions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Midi player functions.
 *
 */


#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include "player.h"

/**
 * \def MUSIC_FILE
 */

#define MUSIC_FILE "score.midi"

//Mix_Music holds the music information.
static Mix_Music* myMus=NULL;




/**
 * \fn void *bmc_play_(void *)
 * \brief This function plays the music contained in an instance of Mix_Music. 
 */
void *bmc_play_(void *v)
{   
    
    // initialize thz sound
    SDL_Init(SDL_INIT_AUDIO);
    
    // open up an audio device.
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)) {
	printf("Unable to open audio!\n");
	exit(1);
    }
    
    // load the music file
    myMus = Mix_LoadMUS(MUSIC_FILE);

    /* play the music */
    Mix_PlayMusic(myMus, 1);

    while (Mix_PlayingMusic() == 1) {
	//While the music isn't finshed, we do something o
	//therwise the programme exits and stops playing 
	SDL_Delay(10);
    }
    if(myMus != NULL)
	free_mix_music();
    return NULL;
}

/**
 * \fn void bmc_play()
 * \brief This function calls bmc_play_() in a new thread. 
 */

void bmc_play()
{   
    if(!Mix_PlayingMusic()) {
	pthread_t t;
	pthread_create(&t,NULL, bmc_play_, NULL);
    }
    else {
	bmc_resume();
    }
}



/**
 * \fn void bmc_pause()
 * \brief This function pauses the music. 
 */

void bmc_pause()
{
    if(Mix_PlayingMusic())
	Mix_PauseMusic();
}

/**
 * \fn void bmc_resume()
 * \brief This function resumes the paused music. 
 */

void bmc_resume()
{
    if(Mix_PausedMusic())
	Mix_ResumeMusic();
}

/**
 * \fn void bmc_stop()
 * \brief This function stop the music. 
 */

void bmc_stop()
{
    if(myMus!=NULL && (Mix_PausedMusic() || Mix_PlayingMusic()))
	free_mix_music();
}

/**
 * \fn void free_mix_music(Mix_Music *myMus)
 * \brief This function delete the instance of Mix_Music 
 * which is created by bmc_play_(). 
 */

void free_mix_music()
{
    Mix_FreeMusic(myMus);
    myMus=NULL;
    Mix_CloseAudio();
    SDL_Quit();
}
