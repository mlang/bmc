#ifndef PLAYER_H
#define PLAYER_H

/**
 * \file player.h
 * \brief Midi player functions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Midi player function prototypes.
 *
 */

void *bmc_play_(void *);

void bmc_play();

void bmc_pause();

void bmc_resume();

void bmc_stop();

void free_mix_music();
#endif
