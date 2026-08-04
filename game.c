#include "window.h"

// gamestate data (playing level, level intro screen, paused, etc)
//
#define PLAYING_LEVEL_ST 0
#define EXITING_LEVEL_ST 1
#define STARTING_LEVEL_ST 2
#define PAUSED_FROM_LEVEL_ST 3

static int gamestate;
static int gamestate_animt;

// entity data (enemies, particles, etc)
//

// TODO implement a "galoomba" (enemy that you can pick up after jumping on it)

#define NULL_ET 0
#define BROKEN_ET 1

typedef struct {

	int type;
	float x, y;
	float dx, dy;

} Entity;

static Entity entities[32] = {}; // first come, first serve

// player data
//
#define PLAYER_COLLIDER_W 24
#define PLAYER_COLLIDER_H 32
#define PLAYER_SPRITE_W 32
#define PLAYER_SPRITE_H 32

#define PLAYER_RUN_ACCEL 0.1
#define PLAYER_RUN_DECEL 0.3
#define PLAYER_MAX_SPEED 3.0
#define PLAYER_JUMP_SPEED -5.0

#define JUMP_LEEWAY 6

typedef struct {

	float x, y;
	float dx, dy;

	int spr_x, spr_y;
	float animt;

	int t_since_jump;     // for input-caching
	int t_since_grounded; // for coyote-time

} Player;

static Player player;

static int candy_count;
static int cam_off;
static int facing_left;

// level data
//
#define LEVEL_HEIGHT 13
#define LEVEL_WIDTH (sizeof(level) / sizeof(int) / LEVEL_HEIGHT)
#define LEVEL_Y_OFFSET (HEIGHT - LEVEL_HEIGHT * 16)

#define IS_COLLIDABLE(i) (i == 1 || i == 2)

static int level_animt;

static int level[] = { // to satisfy spatial locality, level data is stored column-major
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 3, 0, 0, 2, 0, 3, 0, 1, 1,
	0, 0, 0, 0, 3, 0, 0, 2, 0, 3, 0, 1, 1,
	0, 0, 0, 0, 3, 0, 0, 2, 0, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 3, 0, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 3, 0, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 1, 1,
	0, 0, 0, 0, 2, 2, 0, 0, 2, 3, 3, 1, 1,
	0, 0, 0, 0, 2, 2, 0, 0, 2, 3, 3, 1, 1,
	3, 0, 2, 2, 0, 0, 2, 2, 2, 0, 0, 1, 1,
	3, 0, 2, 2, 0, 0, 2, 2, 2, 2, 2, 1, 1,
	3, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	3, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	3, 0, 2, 2, 0, 0, 0, 0, 0, 2, 2, 1, 1,
	3, 0, 2, 2, 0, 0, 0, 0, 0, 2, 2, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
};

// static helper functions
//
static void add_entity(Entity e) { // might fail but doesn't matter

	for (int i = 0; i < sizeof(entities) / sizeof(Entity); i++) {

		if (entities[i].type == NULL_ET) {

			entities[i] = e;
			return;
		}
	}
}

static void get_player_level_aabb(int *start_x, int *start_y, int *end_x, int *end_y) {

	#define FLOOR(v) ((v) >= 0 || (v) == (int) (v) ? ((int) (v)) : ((int) (v) - 1))

	*start_x = FLOOR((player.x - PLAYER_COLLIDER_W / 2) / 16.0);
	*start_y = FLOOR((player.y - PLAYER_COLLIDER_H - LEVEL_Y_OFFSET) / 16.0);

	if (*start_y < 0)
		*start_y = 0;

	*end_x = FLOOR((player.x - 1 + PLAYER_COLLIDER_W / 2) / 16.0);
	*end_y = FLOOR((player.y - 1 - LEVEL_Y_OFFSET) / 16.0);

	if (*end_y > LEVEL_HEIGHT - 1)
		*end_y = LEVEL_HEIGHT - 1;
}

static int player_is_colliding() {

	// colliding with left side of screen?
	if ((int) player.x - PLAYER_COLLIDER_W / 2 < cam_off)
		return 1;

	// colliding with the level?
	int start_x, start_y, end_x, end_y;

	get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

	for (int x = start_x; x <= end_x; x++) {

		for (int y = start_y; y <= end_y; y++) {

			if (IS_COLLIDABLE(level[y + x * LEVEL_HEIGHT]))
				return 1;
		}
	}

	return 0;
}

static void player_move_and_slide() {

	player.x += player.dx;

	if (player_is_colliding()) {

		int start_x, start_y, end_x, end_y;
		get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

		// legalize position
		do {
			player.x -= player.dx * 0.1;
		} while (player_is_colliding());

		player.dx = 0;
	}

	player.y += player.dy;

	if (player_is_colliding()) {

		int start_x, start_y, end_x, end_y;
		get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

		// legalize position
		do {
			player.y -= player.dy * 0.1;
		} while (player_is_colliding());

		// breakable tile breaking (can be modified to allow for breaking downward)
		if (player.dy < 0) {

			int broke_something = 0;

			for (int x = start_x; x <= end_x; x++) {

				if (level[start_y + x * LEVEL_HEIGHT] == 2) {

					level[start_y + x * LEVEL_HEIGHT] = 0;
					broke_something = 1;

					// broken tile particles
					add_entity((Entity) { BROKEN_ET, x * 16, start_y * 16, -1.0, -3.0 });
					add_entity((Entity) { BROKEN_ET, x * 16, start_y * 16,  1.0, -3.0 });
					add_entity((Entity) { BROKEN_ET, x * 16, start_y * 16, -1.0,  0.0 });
					add_entity((Entity) { BROKEN_ET, x * 16, start_y * 16,  1.0,  0.0 });
				}
			}

			if (broke_something) {

				player.dy = player.dy < 0 ? 1.0 : -1.5;
				goto skip_vertical;
			}
		}

		player.t_since_grounded = 0;
		player.dy = 1;

		skip_vertical:

	}
}

static void restart_level() { // e.g. on first start; on death

	// show level intro screen
	gamestate = STARTING_LEVEL_ST;

	// TODO (re)load level

	player.x = 48.0;
	player.y = 248.0;
	player.dx = 0.0;
	player.dy = 0.0;

	player.t_since_jump     = JUMP_LEEWAY;
	player.t_since_grounded = JUMP_LEEWAY;

	candy_count = 0;
	cam_off = 0;
	facing_left = 0;
	
	set_background(100, 200, 255);
}

static void increase_level() { // e.g. on stage win

	// TODO increase level!

	restart_level();
}

static void draw_level_contents() { // player sprite sheet offsets

	// draw level
	// TODO don't draw parts that are obviously off-screen
	for (int i = 0; i < sizeof(level) / sizeof(int); i++) {

		int sprite;

		switch (level[i]) {

			default:
			case 0:
				continue;
			
			case 1: // unbreakable tile
				sprite = 0;
				break;
			
			case 2: // breakable tile
				sprite = 1;
				break;
			
			case 3: // candy
				sprite = 2 + ((level_animt / 16) % 4);
				break;

			case 4: // sign
				sprite = 6;
				break;
		}

		draw_level(sprite, (i / LEVEL_HEIGHT) * 16 - cam_off, 72 + (i % LEVEL_HEIGHT) * 16);
	}

	// process/draw entities
	// TODO move entity processing elsewhere, this function should only do drawing
	for (int i = 0; i < sizeof(entities) / sizeof(Entity); i++) {

		switch (entities[i].type) {

			case NULL_ET:
			default:
				break;

			case BROKEN_ET:
				entities[i].dy += 0.2; // gravity
				entities[i].x += entities[i].dx;
				entities[i].y += entities[i].dy;
				draw_level_flip(17, entities[i].x - cam_off, entities[i].y + LEVEL_Y_OFFSET, ((level_animt + i * 4) / 12) % 2);

				if (entities[i].y > LEVEL_HEIGHT * 16)
					entities[i].type = NULL_ET;
				break;
		}
	}

	// character
	draw_character(0, PLAYER_SPRITE_W, PLAYER_SPRITE_H, player.spr_x, player.spr_y, (int) player.x - PLAYER_SPRITE_W / 2 - cam_off, (int) player.y - PLAYER_SPRITE_H, facing_left);

	// HUD
	draw_level(2, 82, 2); // candy

	char candy_text[] = ".xy";
	candy_text[1] = (candy_count / 10) + '0';
	candy_text[2] = (candy_count % 10) + '0';
	draw_text(candy_text, 97, 8);

	draw_character(0, 16, 16, 96, 64, 3, 2, 0);
	draw_text(".3", 21, 8);

	// draw_text("press start", 3, 24);

	draw_character(1, 16, 16, 96, 64, 3, 18, 0);
	draw_text(".3", 21, 24);

	draw_text("world 1-1", 200, 8);
	draw_text("00002100", 206, 24);
}

// game logic functions
//
void game_init() {

	restart_level();
}

static void update_playing_level(const Input *input) {

	// falling into pit
	if (player.y - PLAYER_COLLIDER_H - LEVEL_Y_OFFSET > LEVEL_HEIGHT * 16) {

		restart_level();
	}

	// update camera offset
	if (player.x + 127.5 > LEVEL_WIDTH * 16) {
		
		cam_off = LEVEL_WIDTH * 16 - WIDTH;

		// we've reached the end of the level (camera has fully scrolled to right side of level)
		facing_left = 0;
		player.dx = PLAYER_MAX_SPEED;
		gamestate = EXITING_LEVEL_ST;

		return;

	} else if (cam_off < player.x - 127.5) {

		cam_off = player.x - 127.5;
	}

	// running
	if (!input->left && !input->right) {

		player.dx *= 0.95;

	} else if (player.t_since_grounded != 0 || !input->down) {

		if (input->left) {
			facing_left = 1;
			player.dx -= player.dx > 0 ? PLAYER_RUN_DECEL : PLAYER_RUN_ACCEL;
			if (-player.dx > PLAYER_MAX_SPEED)
				player.dx = -PLAYER_MAX_SPEED;
		}

		if (input->right) {
			facing_left = 0;
			player.dx += player.dx < 0 ? PLAYER_RUN_DECEL : PLAYER_RUN_ACCEL;
			if (player.dx > PLAYER_MAX_SPEED)
				player.dx = PLAYER_MAX_SPEED;
		}

	} else {
		
		player.dx *= 0.95;
	}

	// cache jump
	if (input->action_a_justchanged && input->action_a)
		player.t_since_jump = 0;

	// jump
	if (player.t_since_grounded < JUMP_LEEWAY && player.t_since_jump < JUMP_LEEWAY) {

		player.dy = PLAYER_JUMP_SPEED + (player.dx < 0 ? player.dx : -player.dx) / PLAYER_MAX_SPEED * 0.5;
		player.t_since_grounded = JUMP_LEEWAY;

		if (!input->action_a) { // the jump was early-released before the jump even started!
			player.dy /= 2;
		}

		player.t_since_jump = JUMP_LEEWAY; // if we don't do this, then we can actually jump twice for one jump press
	}

	// early release jump
	if (player.dy < 0 && input->action_a_justchanged && !input->action_a) {
		player.dy /= 2;
	}

	// gravity
	player.dy += 0.2;

	// apply velocity while respecting collision
	player_move_and_slide();

	// collect candy
	int start_x, start_y, end_x, end_y;
	get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

	for (int x = start_x; x <= end_x; x++) {

		for (int y = start_y; y <= end_y; y++) {

			if (level[y + x * LEVEL_HEIGHT] == 3) {

				candy_count++;
				level[y + x * LEVEL_HEIGHT] = 0;
			}
		}
	}

	// draw

	// determine animation
	if (input->down) {

		player.spr_x = 0;
		player.spr_y = 3 * PLAYER_SPRITE_H;

	} else if (player.t_since_grounded != 0) {

		if (player.dy > -PLAYER_JUMP_SPEED / 2.7) {
			player.spr_x = 2 * PLAYER_SPRITE_W;
			player.spr_y = 2 * PLAYER_SPRITE_H;
		} else if (player.dy < PLAYER_JUMP_SPEED / 2.7) {
			player.spr_x = 0;
			player.spr_y = 2 * PLAYER_SPRITE_H;
		} else {
			player.spr_x = PLAYER_SPRITE_W;
			player.spr_y = 2 * PLAYER_SPRITE_H;
		}
		
	} else if (player.dx == 0 || (!input->left && !input->right && player.dx < PLAYER_RUN_ACCEL && player.dx > -PLAYER_RUN_ACCEL)) {

		// TODO idle animation
		player.spr_x = 0;
		player.spr_y = 0;
		player.animt = 0;

	} else if (facing_left == player.dx > 0) { // turning animation occurs when facing left but moving right (or opposite)
	
		player.spr_x = PLAYER_SPRITE_W;
		player.spr_y = 3 * PLAYER_SPRITE_H;
		player.animt = 0;
	
	} else {

		player.spr_x = (int) player.animt % 4 * PLAYER_SPRITE_W;
		player.spr_y = PLAYER_SPRITE_H;
	}

	draw_level_contents();

	player.animt += (player.dx > 0 ? player.dx : -player.dx) * 0.1;

	level_animt++;
	player.t_since_jump++;
	player.t_since_grounded++;
}

static void update_paused_from_level(const Input *input) {

	draw_level_contents(0, 0);

	draw_text("paused", 111, 8);
}

static void update_ending_level() {

	player.dy += 0.2; // gravity

	player_move_and_slide();

	player.spr_x = (int) player.animt % 4 * PLAYER_SPRITE_W;
	player.spr_y = PLAYER_SPRITE_H;

	draw_level_contents();

	player.animt += player.dx * 0.1;

	if (player.x - 64 > LEVEL_WIDTH * 16)
		increase_level();
}

void game_update(const Input *input) {

	// should move this to inside the static update functions, but idc rn
	if (input->pause && input->pause_justchanged) {

		gamestate = gamestate == PAUSED_FROM_LEVEL_ST ? PLAYING_LEVEL_ST : PAUSED_FROM_LEVEL_ST;
	}

	switch (gamestate) {

		case PLAYING_LEVEL_ST:
			update_playing_level(input);
			break;

		case EXITING_LEVEL_ST:
			update_ending_level();
			break;
		
		case STARTING_LEVEL_ST:
			gamestate = PLAYING_LEVEL_ST; // TODO actually do a little animation
			break;
		
		case PAUSED_FROM_LEVEL_ST:
			update_paused_from_level(input);
			break;
	}
}