// boss3.h - Jefe final del mundo 3: el marciano en OVNI.
//
// Se incluye desde extra_functions.h. No usa enemigos del .ene: el OVNI se dibuja
// con tiles (39,40,41 = platillo, 42 = marciano, 44 = explosion) y las colisiones
// con el jugador y con las balas se hacen aqui a mano.
//
// El OVNI ocupa 3 tiles de ancho y 2 de alto:
//
//        [42]            <- boss_y - 1
//   [39][40][41]         <- boss_y        (boss_x .. boss_x + 2)
//
// Ciclo de combate:
//   B3_TOP  : barre la parte alta de lado a lado BOSS3_TOP_PASSES veces. No se le puede herir.
//   B3_DOWN : baja hasta BOSS3_LOW_Y. No se le puede herir.
//   B3_LOW  : hace UN barrido a la altura del jugador. AQUI ES VULNERABLE.
//   B3_UP   : sube otra vez y vuelta a empezar.

#define BOSS3_TOP_Y			1		// fila del cuerpo en los barridos altos (el marciano va en la fila 0)
#define BOSS3_LOW_Y			8		// fila del cuerpo en el barrido bajo (el marciano va en la 6,
									// la fila por la que corren las balas del jugador)
#define BOSS3_MIN_X			0
#define BOSS3_MAX_X			12		// 15 tiles - 3 de ancho
#define BOSS3_TOP_PASSES	3		// barridos altos antes de bajar
#define BOSS3_FRAMES_TOP	2		// frames entre pasos (1 tile) en los barridos altos
#define BOSS3_FRAMES_LOW	3		// idem en el barrido bajo (mas lento = mas facil de herir)
#define BOSS3_FRAMES_VERT	2		// idem al subir y bajar
#define BOSS3_LIFE			15		// maximo 15: es lo que cabe en la barra de vida
#define BOSS3_DYING_FRAMES	24		// duracion de la explosion (multiplo de 4)
#define BOSS3_FLASH_FRAMES	4		// frames que parpadea al recibir un impacto
#define BOSS3_PLAYER_X		2		// donde aparece el jugador (tile)
#define BOSS3_PLAYER_Y		6		// fila 6 = de pie sobre las plataformas de la fila 7

#define B3_TOP		0
#define B3_DOWN		1
#define B3_LOW		2
#define B3_UP		3
#define B3_DYING	10
#define B3_DEAD		99
#define B3_ALIGN	4

unsigned char boss3_state = B3_TOP;
unsigned char boss3_dir = 0;		// 0 = izquierda, 1 = derecha
unsigned char boss3_side = 0;		// lado por el que baja: 0 = izquierda, 1 = derecha
unsigned char boss3_pass = 0;
unsigned char boss3_tick = 0;
unsigned char boss3_limit = 0;
unsigned char boss3_flash = 0;
unsigned char boss3_i = 0;
unsigned char boss3_cx = 0;
unsigned char boss3_cy = 0;

// Dibuja un tile relativo al OVNI: dx 0..2 a la derecha de boss_x, up filas por encima de boss_y.
void boss3_put (unsigned char dx, unsigned char up, unsigned char t) {
	_x = boss_x + dx;
	_y = boss_y - up;
	_t = t;
	draw_invalidate_coloured_tile_gamearea ();
}

// Repinta el fondo original de una celda (el OVNI nunca toca map_buff).
void boss3_restore (unsigned char dx, unsigned char up) {
	_x = boss_x + dx;
	_y = boss_y - up;
	_t = qtile (_x, _y);
	draw_invalidate_coloured_tile_gamearea ();
}

void boss3_draw (void) {
	boss3_put (0, 0, 39);
	boss3_put (1, 0, 40);
	boss3_put (2, 0, 41);
	boss3_put (1, 1, boss3_flash ? 44 : 42);
}

void boss3_explode (void) {
	boss3_put (0, 0, 44);
	boss3_put (1, 0, 44);
	boss3_put (2, 0, 44);
	boss3_put (1, 1, 44);
}

void boss3_erase (void) {
	boss3_restore (0, 0);
	boss3_restore (1, 0);
	boss3_restore (2, 0);
	boss3_restore (1, 1);
}

void boss3_move (signed char dx, signed char dy) {
	boss3_erase ();
	boss_x = boss_x + dx;
	boss_y = boss_y + dy;
	boss3_draw ();
}

// La pantalla 0 del mundo 3 trae tiles 39..43 sueltos. Son letales y se repintan en cada
// draw_scr, asi que los quitamos del mapa en memoria al entrar en la pantalla.
void boss3_clear_map (void) {
	for (boss3_cy = 0; boss3_cy < 10; boss3_cy ++) {
		for (boss3_cx = 0; boss3_cx < 15; boss3_cx ++) {
			_t = qtile (boss3_cx, boss3_cy);
			if (_t >= 39 && _t <= 43) {
				_x = boss3_cx;
				_y = boss3_cy;
				_t = 0;
				_n = 0;
				update_tile ();
			}
		}
	}
	if (boss3_state != B3_DEAD && boss_y >= 1 && boss_x <= BOSS3_MAX_X) boss3_draw ();
}

// Pone al jefe (y al jugador) en la posicion de inicio. Al entrar en la pelea y al reintentar.
void boss3_init (void) {
	boss3_state = B3_TOP;
	boss3_dir = 0;				// empieza yendo a la izquierda
    boss3_side = 0;
	boss3_pass = 0;
	boss3_tick = 0;
	boss3_flash = 0;
	boss_action = 0;
	boss_x = BOSS3_MAX_X;
	boss_y = BOSS3_TOP_Y;
	_en_life_boss = BOSS3_LIFE;

	// El jugador empieza de pie sobre la plataforma.
	gpx = BOSS3_PLAYER_X << 4; p_x = gpx << 6;
	gpy = BOSS3_PLAYER_Y << 4; p_y = gpy << 6;
}

void boss3_die (void) {
	boss3_state = B3_DYING;
	boss3_tick = BOSS3_DYING_FRAMES;
	boss3_flash = 0;
	boss_action = 10;
	timer_on = 0;
	points += 500;
	print_points ();
	check_points ();
	PLAY_SOUND (SFX_KILL_ENEMY_SHOOT);
	PLAY_MUSIC (1);
}

// Un impacto del jugador. Solo se llama en B3_LOW.
void boss3_hit (void) {
	if (_en_life_boss) _en_life_boss --;
	draw_sub_boss_life ();
	PLAY_SOUND (SFX_HIT_ENEMY);
	if (_en_life_boss == 0) {
		boss3_die ();
	} else {
		boss3_flash = BOSS3_FLASH_FRAMES;
		boss3_draw ();
	}
}

// Balas del jugador (bullets_who == 1) contra el OVNI. Solo es vulnerable en B3_LOW.
void boss3_check_hits (void) {
	for (boss3_i = 0; boss3_i < MAX_BULLETS; boss3_i ++) {
		if (bullets_estado [boss3_i] && bullets_who [boss3_i] == 1) {
			boss3_cx = (bullets_x [boss3_i] + 3) >> 4;
			boss3_cy = (bullets_y [boss3_i] + 3) >> 4;
			if (boss3_cx >= boss_x && boss3_cx <= boss_x + 2 &&
				(boss3_cy == boss_y || boss3_cy + 1 == boss_y)) {
				bullets_estado [boss3_i] = 0;
				bullets_who [boss3_i] = 0;
				if (shoots) shoots --;
				boss3_hit ();
				if (boss3_state == B3_DYING) return;
			}
		}
	}
}

// Un paso horizontal (1 tile). Devuelve 1 cuando termina el barrido (llega al borde).
unsigned char boss3_sweep (void) {
	if (boss3_dir == 0) {
		if (boss_x > BOSS3_MIN_X) boss3_move (-1, 0);
		if (boss_x == BOSS3_MIN_X) { boss3_dir = 1; return 1; }
	} else {
		if (boss_x < BOSS3_MAX_X) boss3_move (1, 0);
		if (boss_x == BOSS3_MAX_X) { boss3_dir = 0; return 1; }
	}
	return 0;
}

void play_boss3 (void) {
	if (boss3_state == B3_DEAD) return;

	// Explosion final; al acabar se activa la pantalla de "level complete".
	if (boss3_state == B3_DYING) {
		boss3_tick --;
		if (boss3_tick == 0) {
			boss3_erase ();
			boss3_state = B3_DEAD;
			boss_action = 99;
			stage_clear_animation = 1;
		} else if ((boss3_tick & 3) == 0) {
			if (boss3_tick & 4) boss3_explode (); else boss3_draw ();
		}
		return;
	}

	// Parpadeo tras un impacto.
	if (boss3_flash) {
		boss3_flash --;
		if (boss3_flash == 0) boss3_draw ();
	}

	// Solo se puede herir durante el barrido bajo.
	if (boss3_state == B3_LOW) {
		boss3_check_hits ();
		if (boss3_state == B3_DYING) return;
	}

	// Tocar al OVNI mata en cualquier fase.
	if (p_estado == EST_NORMAL &&
		p_tx >= boss_x && p_tx <= boss_x + 2 &&
		(p_ty == boss_y || p_ty + 1 == boss_y)) {
		p_killme = 1;
	}

	// Velocidad segun la fase.
	if (boss3_state == B3_TOP || boss3_state == B3_ALIGN) boss3_limit = BOSS3_FRAMES_TOP;
	else if (boss3_state == B3_LOW) boss3_limit = BOSS3_FRAMES_LOW;
	else boss3_limit = BOSS3_FRAMES_VERT;

	boss3_tick ++;
	if (boss3_tick < boss3_limit) return;
	boss3_tick = 0;

	if (boss3_state == B3_TOP) {
		if (boss3_sweep ()) {
			boss3_pass ++;
			if (boss3_pass >= BOSS3_TOP_PASSES) {
				boss3_pass = 0;
				// Elige al azar por que lado baja.
				boss3_side = (rand () & 0x10) ? 1 : 0;
				boss3_state = B3_ALIGN;
			}
		}
	} else if (boss3_state == B3_ALIGN) {
		// Se coloca arriba en el borde elegido; el barrido bajo ira hacia el lado contrario.
		if (boss3_side == 0) {
			if (boss_x > BOSS3_MIN_X) boss3_move (-1, 0);
			if (boss_x == BOSS3_MIN_X) { boss3_dir = 1; boss3_state = B3_DOWN; }
		} else {
			if (boss_x < BOSS3_MAX_X) boss3_move (1, 0);
			if (boss_x == BOSS3_MAX_X) { boss3_dir = 0; boss3_state = B3_DOWN; }
		}
	} else if (boss3_state == B3_DOWN) {
		if (boss_y < BOSS3_LOW_Y) boss3_move (0, 1);
		if (boss_y == BOSS3_LOW_Y) boss3_state = B3_LOW;
	} else if (boss3_state == B3_LOW) {
		// Un solo barrido, hacia el lado contrario al que estaba.
		if (boss3_sweep ()) boss3_state = B3_UP;
	} else if (boss3_state == B3_UP) {
		if (boss_y > BOSS3_TOP_Y) boss3_move (0, -1);
		if (boss_y == BOSS3_TOP_Y) {
			boss3_pass = 0;
			boss3_state = B3_TOP;
		}
	}
}