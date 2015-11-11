/* *
 * Hochschule RheinMain
 * Fachbereich DCSM
 * Studienbereich Informatik
 *
 * Lehrveranstaltung Echzeitverarbeitung
 *
 * Sommersemester 2015
 *
 * Api zum steuern eines Schrittmotors
 *
 * Uebersetzten mit:
 * gcc -std=c99 -Wall PROGRAMMNAME.c -o PROGRAMMNAME -lpthread -D_GNU_SOURCE 
 * -lrt -I/usr/local/include -L/usr/local/lib -lwiringPi
 * 
 * License: GPLv2
 *
 * von Harald Heckmann
 */

#ifndef __STEPLIB__H__
#define __STEPLIB__H__

#define BACKWARDS 217
#define FORWARDS 218

// init steplib, steps:
// init wiringPi
// declare pins to use and pinmode
// write 0 to all pins
// set prio to 0 if you do not want to set a priority
// returns errno for perror
int init_steplib(unsigned short r1, unsigned short r2,\
			unsigned short l1, unsigned short l2, unsigned int prio);


// do fullsteps
// 1.8° * steps
// dir = direction = BACKWARDS || FORWARDS
void fullstep(unsigned long long nanotime, unsigned long long steps, unsigned char dir);

// do halfsteps
// 1.8° * steps
// dir = direction = BACKWARDS || FORWARDS
void halfstep(unsigned long long nanotime, unsigned long long steps, unsigned char dir);

// uninit (set all pins to 0 if they have been initialized)
void exit_steplib();

#endif //__STEPLIB__H__
