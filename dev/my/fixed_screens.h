// MTE MK1 (la Churrera) v5.10
// Copyleft 2010-2014, 2020-2023 by the Mojon Twins

#ifndef MODE_128K
	void lame_sound (void) {
		gpit = 4; do {
			beep_fx (rda);
			beep_fx (rdb);
		} while (-- gpit);
		beep_fx (9);
	}
#endif

void game_ending (void) {
	sp_UpdateNow();
	blackout ();
	#ifdef MODE_128K
		// Resource 2 = ending
		PLAY_MUSIC (13);
		unsigned char c = 0;
		for (c = 0; c < 3; c++) {
			get_resource (ending_screens[c], 16384);
			espera_activa (650);
			cortina ();
			blackout ();
		}
		STOP_SOUND();
		blackout_area ();	
	#else
		#asm
			ld hl, _s_ending
			ld de, 16384
			call depack
		#endasm
	#endif

	#ifdef MODE_128K
	#else
		rda = 7; rdb = 2;
		lame_sound ();
	#endif
	
	#ifdef MODE_128K
		espera_musica (3000);
	#else
		espera_activa (500);
	#endif
}

void game_over (void) {
	
	clear_sprites();
	clear_gamezone();

	sp_UpdateNow();
    get_resource (INTRO6_BIN, 16384);
	espera_activa (200);
	cortina();

	
	//_x = 10; _y = 11; _t = 79; _gp_gen = spacer; print_str ();
	_x = 10; _y = 12; _t = 3; _gp_gen = (unsigned char *)("  GAME OVER  "); print_str ();
	//_x = 10; _y = 13; _t = 79; _gp_gen = spacer; print_str ();
	sp_UpdateNow ();

	#ifdef MODE_128K
	#else
		rda = 7; rdb = 2;
		lame_sound ();
	#endif

	#ifdef MODE_128K
	espera_musica (800);
		espera_activa (50);
	#else
		espera_activa (800);
	#endif
}

#if defined(TIMER_ENABLE) && defined(SHOW_TIMER_OVER)
	void time_over (void) {
		_x = 10; _y = 11; _t = 79; _gp_gen = spacer; print_str ();
		_x = 10; _y = 12; _t = 79; _gp_gen = (unsigned char *)("  TIME'S UP "); print_str ();
		_x = 10; _y = 13; _t = 79; _gp_gen = spacer; print_str ();
		sp_UpdateNowEx (0);
			
		#ifdef MODE_128K
		#else
			rda = 1; rdb = 2;
		lame_sound ();
		#endif
		
		espera_activa (80);
	}
#endif

#ifdef PAUSE_ABORT
	void pause_screen (void) {
		_x = 10; _y = 11; _t = 79; _gp_gen = spacer; print_str ();
		_x = 10; _y = 12; _t = 79; _gp_gen = (unsigned char *)("   PAUSED   "); print_str ();
		_x = 10; _y = 13; _t = 79; _gp_gen = spacer; print_str ();
		sp_UpdateNowEx (0);
	}
#endif

