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

#include "../include/steplib.h"
#include <wiringPi.h>
#include <time.h>


static unsigned short a1 = 0;
static unsigned short a2 = 0;
static unsigned short b1 = 0;
static unsigned short b2 = 0;
static unsigned char sl_init = 0;
static unsigned char state = 0;

// here begins a code segment written by Kai Beckmann
static void sleep_util(struct timespec* ts, unsigned long long delay) {
	ts->tv_nsec += delay;

	while (ts->tv_nsec >= 1000 * 1000 * 1000) {
		ts->tv_nsec -= 1000 * 1000 * 1000;
		ts->tv_sec++;
	}

	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, ts, NULL);
}
// end code segment

static void reset() {
	if (1 != sl_init) return;
	digitalWrite(a1, 0);
	digitalWrite(a2, 0);
	digitalWrite(b1, 0);
	digitalWrite(b2, 0);
	state = 0;
}


int init_steplib(unsigned short phasea1, unsigned short phasea2,\
			unsigned short phaseb1, unsigned short phaseb2, unsigned int prio) {
	int err;
	a1 = phasea1;
	a2 = phasea2;
	b1 = phaseb1;
	b2 = phaseb2;

	err = wiringPiSetupGpio();
	if (0 != err) return err;

	// unsigned
	if (0 != prio) {
		err = piHiPri(prio);
		if (0 != err) return err; 
	}

	pinMode(a1, OUTPUT);
	pinMode(a2, OUTPUT);
	pinMode(b1, OUTPUT);
	pinMode(b2, OUTPUT);

	reset();
	sl_init = 1;
	state = 0;

	return 0;
}

void exit_steplib() {
	if (0 != sl_init) reset();
}

void fullstep(unsigned long long nanotime, unsigned long long steps, unsigned char dir) {
	if (1 != sl_init) return;
	struct timespec ts;
	unsigned long long delay = (unsigned long long) (nanotime/steps);

	while (steps > 0) {
		if (state == 0 && steps > 0) {
			// step 1
			digitalWrite(a1, 1);
			digitalWrite(a2, 0);
			digitalWrite(b1, 1);
			digitalWrite(b2, 0);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = ((dir == FORWARDS) ? 1 : 3);
			--steps;
		}

		if (state == 1 && steps > 0) {
			// step 2
			digitalWrite(a1, 1);
			digitalWrite(a2, 0);
			digitalWrite(b1, 0);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = ((dir == FORWARDS) ? 2 : 0);
			--steps;
		}

		if (state == 2 && steps > 0) {
			// step 3
			digitalWrite(a1, 0);
			digitalWrite(a2, 1);
			digitalWrite(b1, 0);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = ((dir == FORWARDS) ? 3 : 1);
			--steps;
		}

		if (state == 3 && steps > 0) {
			// step 4
			digitalWrite(a1, 0);
			digitalWrite(a2, 1);
			digitalWrite(b1, 1);
			digitalWrite(b2, 0);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = ((dir == FORWARDS) ? 0 : 2);
			--steps;
		}
	}
}

void halfstep_bw(unsigned long long delay, unsigned long long steps) {
	struct timespec ts;

	while (steps > 0) {
		if (state == 0 && steps > 0) {
			// step 1
			digitalWrite(a1, 1);
			digitalWrite(a2, 0);
			digitalWrite(b1, 1);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);

			digitalWrite(a1, 1);
			digitalWrite(a2, 0);
			digitalWrite(b1, 1);
			digitalWrite(b2, 0);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = 3; --steps;
		}

		if (state == 1 && steps > 0) {
			// step 2
			digitalWrite(a1, 1);
			digitalWrite(a2, 1);
			digitalWrite(b1, 0);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);

			digitalWrite(a1, 1);
			digitalWrite(a2, 0);
			digitalWrite(b1, 0);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			--state; --steps;
		}

		if (state == 2 && steps > 0) {
			// step 3
			digitalWrite(a1, 0);
			digitalWrite(a2, 1);
			digitalWrite(b1, 1);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);

			digitalWrite(a1, 0);
			digitalWrite(a2, 1);
			digitalWrite(b1, 0);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			--state; --steps;
		}

		if (state == 3 && steps > 0) {
			// step 4
			digitalWrite(a1, 1);
			digitalWrite(a2, 1);
			digitalWrite(b1, 1);
			digitalWrite(b2, 0);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);

			digitalWrite(a1, 0);
			digitalWrite(a2, 1);
			digitalWrite(b1, 1);
			digitalWrite(b2, 0);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			--state; --steps;
		}
	}
}

void halfstep(unsigned long long nanotime, unsigned long long steps, unsigned char dir) {
	if (1 != sl_init) return;
	struct timespec ts;
	// steps*2 because each step (1.8°) is now devided into two steps
	unsigned long long delay = (unsigned long long) (nanotime/(steps*2));

	if (dir == BACKWARDS) {
		halfstep_bw(delay, steps);
		return;
	}

	while (steps > 0) {
		if (state == 0 && steps > 0) {
			// step 1
			digitalWrite(a1, 1);
			digitalWrite(a2, 0);
			digitalWrite(b1, 1);
			digitalWrite(b2, 0);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);

			digitalWrite(a1, 1);
			digitalWrite(a2, 0);
			digitalWrite(b1, 1);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = 1;
			--steps;
		}

		if (state == 1 && steps > 0) {
			// step 2
			digitalWrite(a1, 1);
			digitalWrite(a2, 0);
			digitalWrite(b1, 0);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);

			digitalWrite(a1, 1);
			digitalWrite(a2, 1);
			digitalWrite(b1, 0);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = 2;
			--steps;
		}

		if (state == 2 && steps > 0) {
			// step 3
			digitalWrite(a1, 0);
			digitalWrite(a2, 1);
			digitalWrite(b1, 0);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);

			digitalWrite(a1, 0);
			digitalWrite(a2, 1);
			digitalWrite(b1, 1);
			digitalWrite(b2, 1);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = 3;
			--steps;
		}

		if (state == 3 && steps > 0) {
			// step 4
			digitalWrite(a1, 0);
			digitalWrite(a2, 1);
			digitalWrite(b1, 1);
			digitalWrite(b2, 0);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);

			digitalWrite(a1, 1);
			digitalWrite(a2, 1);
			digitalWrite(b1, 1);
			digitalWrite(b2, 0);

			clock_gettime(CLOCK_MONOTONIC, &ts);
			sleep_util(&ts, delay);
			state = 0;
			--steps;
		}
	}
}
