/* *
 * Hochschule RheinMain
 * Fachbereich DCSM
 * Studienbereich Informatik
 *
 * Lehrveranstaltung Echzeitverarbeitung
 *
 * Sommersemester 2015
 *
 * Programm zum testen der Motorsteuerungsapi und der Grenzen des Motors
 *
 * Uebersetzten mit:
 * gcc -std=c99 -Wall PROGRAMMNAME.c -o PROGRAMMNAME -lpthread -D_GNU_SOURCE 
 * -lrt -I/usr/local/include -L/usr/local/lib -lwiringPi
 * 
 * License: GPLv2
 *
 * von Harald Heckmann
 */

#include "../include/steplib.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#define error_macro(msg) {\
	perror(msg);\
	exit(EXIT_FAILURE);\
}

// 180° H-Bridge adapted
#define RIGHT_GO 17
#define RIGHT_DIR 4
#define LEFT_GO 10
#define LEFT_DIR 25


#define PHASE_A1 LEFT_GO
#define PHASE_A2 RIGHT_DIR
#define PHASE_B1 RIGHT_GO
#define PHASE_B2 LEFT_DIR


static int err = 0;
static struct sigaction act;

static void sighandler(int sig)
{
	printf("\nshutting down motor and quitting.\n");

	exit_steplib();
	exit(EXIT_FAILURE);
}

int main() {
	// init signalhandler
	// initialize sigaction struct
	memset (&act, '\0', sizeof(act));
 
	// define function to be called
	act.sa_handler = sighandler;
	act.sa_flags = 0;

	err = sigemptyset(&act.sa_mask);
	if (err < 0)
	{
		perror("sigemptyset");
	}

	err = sigaction(SIGALRM, &act, NULL);
	if (err < 0) {
		perror ("sigaction");
		return 1;
	}

	err = sigaction(SIGTERM, &act, NULL);
	if (err < 0) {
		perror ("sigaction");
		return 1;
	}

	err = sigaction(SIGINT, &act, NULL);
	if (err < 0) {
		perror ("sigaction");
		return 1;
	}

	// offer the user to unload modules which block LEFT
	// this program needs to be executed with sudo permissions anyways
	err = printf("Unload w1_therm, w1_gpio and wire so you can use \
LEFT_GO and LEFT_DIR?\nit is highly recommended\n\
decision (y/n): ");
	fflush(stdout);

	if ('y' == getchar()) {
		err = system("sudo rmmod w1_therm");
		printf("w1_therm unloaded\n");

		err = system("sudo rmmod w1_gpio");
		printf("w1_gpio unloaded\n");

		err = system("sudo rmmod wire");
		printf("wire unloaded\n");

		// fix a strange bug
		(void) getchar();
	}

	// init stepper lib
	err = init_steplib(PHASE_A1, PHASE_A2, PHASE_B1, PHASE_B2, 98);
	if (0 != err) error_macro("steplib");

	// begin the stepping
	while (1) {
		printf("step? (n or CTRL-C = exit): ");
		fflush(stdout);

		if ('n' != getchar()) {
			printf("fullstep, forwards, 200 * 1.8° = 360°\n");
			fullstep(650000000, 200, FORWARDS);
			printf("fullstep, backwards, 200 * 1.8° = 360°\n");
			fullstep(650000000, 200, BACKWARDS);
			printf("halfstep, forwards, 200 * 1.8° = 360°\n");
			halfstep(650000000, 200, FORWARDS);
			printf("halfstep, backwards, 200 * 1.8° = 360°\n");
			halfstep(650000000, 200, BACKWARDS);
		}
	}

	// reset the pins to 0 (LOW) again
	exit_steplib();
	exit(EXIT_SUCCESS);
}
