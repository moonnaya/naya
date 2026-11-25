// MTE MK1 (la Churrera) v5.0
// Copyleft 2010-2014, 2020 by the Mojon Twins

#ifdef DEBUG_START_AT_BOSS3
	// Mismo arranque que cuando se llega al jefe 3 jugando normal.
	boss = 1;
	n_pant = 0;
	timer_t = 60;
	#include "my/level_boss_screen.h"
	boss3_init ();
	get_resource (MARCO_BIN, 16384);
#endif