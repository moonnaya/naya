// boss3.h - Jefe final del mundo 3: el marciano en OVNI.   (version COMPACTA)
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
//   B3_TOP   : barre la parte alta de lado a lado BOSS3_TOP_PASSES veces. No se le puede herir.
//   B3_ALIGN : elige al azar izquierda o derecha y se coloca arriba en ese borde.
//   B3_DOWN  : baja por ese borde hasta BOSS3_LOW_Y (a ras de suelo). No se le puede herir.
//   B3_LOW   : hace UN barrido hacia el lado contrario. AQUI ES VULNERABLE.
//   B3_UP    : sube otra vez y vuelta a empezar.
//
// Esta version esta escrita para ocupar poca memoria (el MK1 en 128K va muy justo
// y el codigo no puede pasar de $F000): una unica rutina de pintado y una maquina de
// estados basada en "objetivos" (boss3_tx, boss3_ty).

#define BOSS3_TOP_Y			1		// fila del cuerpo en los barridos altos (el marciano va en la fila 0)
#define BOSS3_LOW_Y			8		// fila del cuerpo en el barrido bajo, a ras de suelo
#define BOSS3_MIN_X			0
#define BOSS3_MAX_X			12		// 15 tiles - 3 de ancho
#define BOSS3_TOP_PASSES	3		// barridos altos antes de bajar
#define BOSS3_FRAMES_TOP	2		// frames entre pasos (1 tile) en los barridos altos
#define BOSS3_FRAMES_LOW	3		// idem en el barrido bajo (mas lento = mas facil de herir)
#define BOSS3_FRAMES_VERT	2		// idem al subir y bajar
#define BOSS3_LIFE			15		// maximo 15: es lo que cabe en la barra de vida
#define BOSS3_DYING_FRAMES	24		// duracion de la explosion
#define BOSS3_PLAYER_X		2		// donde aparece el jugador (tile)
#define BOSS3_PLAYER_Y		6		// fila 6 = de pie sobre las plataformas de la fila 7

#define B3_TOP		0
#define B3_ALIGN	1
#define B3_DOWN		2
#define B3_LOW		3
#define B3_UP		4
#define B3_DYING	10
#define B3_DEAD		99

#define BOSS3_ENEMY_FIRST	4
#define BOSS3_ENEMY_SECS	6
#define BOSS3_ENEMY_OFF		20

unsigned char boss3_state = B3_TOP;
unsigned char boss3_tx = 0;			// objetivo horizontal (columna)
unsigned char boss3_ty = 0;			// objetivo vertical (fila)
unsigned char boss3_pass = 0;
unsigned char boss3_tick = 0;
unsigned char boss3_k = 0;			// contador de celdas del pintado
unsigned char boss3_i = 0;			// contador de balas
unsigned char boss3_cx = 0;
unsigned char boss3_cy = 0;

unsigned char boss3_wait = 0;
unsigned char boss3_last = 0;
unsigned char boss3_enemy [10] = { 224, 96, 0, 96, 224, 96, 254, 0, 1, 2 };

void boss3_spawn (void) {
	unsigned char *p;
	p = (unsigned char *) malotes + BOSS3_ENEMY_OFF;
	for (boss3_k = 0; boss3_k < 10; boss3_k ++) p [boss3_k] = boss3_enemy [boss3_k];
	if (gpx >= 112) { p [0] = 0; p [6] = 2; }
	en_an_state [2] = 0;
}

void boss3_enemy_hide (void) {
	unsigned char *p;
	p = (unsigned char *) malotes + BOSS3_ENEMY_OFF;
	p [0] = p [1] = 255;
	p [8] = 0;
}

// Pinta las 4 celdas del OVNI. mode: 0 = dibujar, 1 = explosion, 2 = restaurar el fondo.
// Celdas: 0,1,2 = platillo (boss_x + k, boss_y); 3 = marciano (boss_x + 1, boss_y - 1).
// Tiles: 39 + k  ->  39, 40, 41, 42.
void boss3_paint (unsigned char mode) {
	for (boss3_k = 0; boss3_k < 4; boss3_k ++) {
		_x = boss_x + (boss3_k == 3 ? 1 : boss3_k);
		_y = boss_y - (boss3_k == 3);
		if (mode == 2) _t = qtile (_x, _y);
		else if (mode) _t = 44;
		else _t = 39 + boss3_k;
		draw_invalidate_coloured_tile_gamearea ();
	}
}

void boss3_move (signed char dx, signed char dy) {
	boss3_paint (2);
	boss_x = boss_x + dx;
	boss_y = boss_y + dy;
	boss3_paint (0);
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
	if (boss3_state != B3_DEAD && boss_y && boss_x <= BOSS3_MAX_X) boss3_paint (0);
}

// Pone al jefe (y al jugador) en la posicion de inicio. Al entrar en la pelea y al reintentar.
void boss3_init (void) {
	boss3_state = B3_TOP;
	boss3_pass = 0;
	boss3_tick = 0;
	boss_action = 0;
	boss_x = BOSS3_MAX_X;
	boss_y = BOSS3_TOP_Y;
	boss3_tx = BOSS3_MIN_X;		// empieza yendo a la izquierda
	boss3_ty = BOSS3_TOP_Y;
	_en_life_boss = BOSS3_LIFE;

    boss3_enemy_hide ();
	boss3_wait = BOSS3_ENEMY_FIRST;
	boss3_last = timer_t;

	// El jugador empieza de pie sobre la plataforma.
	gpx = BOSS3_PLAYER_X << 4; p_x = gpx << 6;
	gpy = BOSS3_PLAYER_Y << 4; p_y = gpy << 6;
}

// Un impacto del jugador. Solo se llama en B3_LOW.
void boss3_hit (void) {
	if (_en_life_boss) _en_life_boss --;
	draw_sub_boss_life ();
	PLAY_SOUND (SFX_HIT_ENEMY);
	if (_en_life_boss == 0) {
		boss3_state = B3_DYING;
		boss3_tick = BOSS3_DYING_FRAMES;
		boss_action = 10;
		timer_on = 0;
		points += 500;
		print_points ();
		check_points ();
		PLAY_SOUND (SFX_KILL_ENEMY_SHOOT);
		PLAY_MUSIC (1);
        boss3_enemy_hide ();
		boss3_paint (1);
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

// El OVNI ha llegado a su objetivo: decide la siguiente fase y el siguiente objetivo.
// (boss_x vale siempre MIN o MAX cuando se llama con el OVNI en TOP/DOWN/LOW/UP.)
void boss3_next (void) {
	if (boss3_state == B3_TOP) {
		boss3_pass ++;
		if (boss3_pass >= BOSS3_TOP_PASSES) {
			boss3_pass = 0;
			boss3_state = B3_ALIGN;
			// Elige al azar por que lado baja.
			boss3_tx = (rand () & 0x10) ? BOSS3_MAX_X : BOSS3_MIN_X;
		} else {
			boss3_tx = BOSS3_MIN_X + BOSS3_MAX_X - boss_x;		// el otro borde
		}
	} else if (boss3_state == B3_ALIGN) {
		boss3_state = B3_DOWN;
		boss3_ty = BOSS3_LOW_Y;
	} else if (boss3_state == B3_DOWN) {
		boss3_state = B3_LOW;
		boss3_tx = BOSS3_MIN_X + BOSS3_MAX_X - boss_x;			// barrido hacia el lado contrario
	} else if (boss3_state == B3_LOW) {
		boss3_state = B3_UP;
		boss3_ty = BOSS3_TOP_Y;
	} else {
		boss3_state = B3_TOP;
		boss3_tx = BOSS3_MIN_X + BOSS3_MAX_X - boss_x;
	}
}

void play_boss3 (void) {
	if (boss3_state == B3_DEAD) return;

	// Explosion final; al acabar se activa la pantalla de "level complete".
	if (boss3_state == B3_DYING) {
		boss3_tick --;
		if (boss3_tick == 0) {
			boss3_paint (2);
			boss3_state = B3_DEAD;
			boss_action = 99;
			stage_clear_animation = 1;
		}
		return;
	}

    if (boss3_last != timer_t) {
        boss3_last = timer_t;
        if (boss3_wait) boss3_wait --;
	}
	if (((unsigned char *) malotes) [BOSS3_ENEMY_OFF + 8] == 1) boss3_wait = BOSS3_ENEMY_SECS;
	else if (boss3_wait == 0) boss3_spawn ();

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
	boss3_k = BOSS3_FRAMES_TOP;
	if (boss3_state == B3_LOW) boss3_k = BOSS3_FRAMES_LOW;
	else if (boss3_state == B3_DOWN || boss3_state == B3_UP) boss3_k = BOSS3_FRAMES_VERT;

	boss3_tick ++;
	if (boss3_tick < boss3_k) return;
	boss3_tick = 0;

	// Un paso hacia el objetivo (primero en horizontal, luego en vertical).
	if (boss_x < boss3_tx) boss3_move (1, 0);
	else if (boss_x > boss3_tx) boss3_move (-1, 0);
	else if (boss_y < boss3_ty) boss3_move (0, 1);
	else if (boss_y > boss3_ty) boss3_move (0, -1);
	else boss3_next ();
}