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

static int player_sprite_y_offset = 0;

static float p_x, p_y;
static float p_dx, p_dy;

static int candy_count;
static int cam_off;
static int facing_left;

#define JUMP_LEEWAY 6
static int t_since_jump;     // for input-caching
static int t_since_grounded; // for coyote-time

static float p_animt;

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

	*start_x = FLOOR((p_x - PLAYER_COLLIDER_W / 2) / 16.0);
	*start_y = FLOOR((p_y - PLAYER_COLLIDER_H - LEVEL_Y_OFFSET) / 16.0);

	if (*start_y < 0)
		*start_y = 0;

	*end_x = FLOOR((p_x - 1 + PLAYER_COLLIDER_W / 2) / 16.0);
	*end_y = FLOOR((p_y - 1 - LEVEL_Y_OFFSET) / 16.0);

	if (*end_y > LEVEL_HEIGHT - 1)
		*end_y = LEVEL_HEIGHT - 1;
}

static int player_is_colliding() {

	// colliding with left side of screen?
	if ((int) p_x - PLAYER_COLLIDER_W / 2 < cam_off)
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

	p_x += p_dx;

	if (player_is_colliding()) {

		int start_x, start_y, end_x, end_y;
		get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

		// legalize position
		do {
			p_x -= p_dx * 0.1;
		} while (player_is_colliding());

		p_dx = 0;

		skip_horizontal:
	}

	p_y += p_dy;

	if (player_is_colliding()) {

		int start_x, start_y, end_x, end_y;
		get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

		// legalize position
		do {
			p_y -= p_dy * 0.1;
		} while (player_is_colliding());

		// breakable tile breaking (can be modified to allow for breaking downward)
		if (p_dy < 0) {

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

				p_dy = p_dy < 0 ? 1.0 : -1.5;
				goto skip_vertical;
			}
		}

		t_since_grounded = 0;
		p_dy = 1;

		skip_vertical:

	}
}

static void restart_level() { // e.g. on first start; on death

	// show level intro screen
	gamestate = STARTING_LEVEL_ST;

	// TODO (re)load level

	p_x = 48.0;
	p_y = 248.0;
	p_dx = 0.0;
	p_dy = 0.0;

	t_since_jump     = JUMP_LEEWAY;
	t_since_grounded = JUMP_LEEWAY;

	candy_count = 0;
	cam_off = 0;
	facing_left = 0;
	
	set_background(100, 200, 255);
}

static void increase_level() { // e.g. on stage win

	// TODO increase level!

	restart_level();
}

static void draw_level_contents(int anim_x, int anim_y) { // player sprite sheet offsets

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
	draw_character(PLAYER_SPRITE_W, PLAYER_SPRITE_H, anim_x, anim_y + player_sprite_y_offset, (int) p_x - PLAYER_SPRITE_W / 2 - cam_off, (int) p_y - PLAYER_SPRITE_H, facing_left);

	// HUD
	draw_level(2, 2, 2);

	char candy_text[] = ".xy"; // TODO "world 1-1"
	candy_text[1] = (candy_count / 10) + '0';
	candy_text[2] = (candy_count % 10) + '0';
	draw_text(candy_text, 17, 8);
}

// game logic functions
//
void game_init() {

	restart_level();
}

static void update_playing_level(const Input *input) {

	// falling into pit
	if (p_y - PLAYER_COLLIDER_H - LEVEL_Y_OFFSET > LEVEL_HEIGHT * 16) {

		restart_level();
	}

	// update camera offset
	if (p_x + 127.5 > LEVEL_WIDTH * 16) {
		
		cam_off = LEVEL_WIDTH * 16 - WIDTH;

		// we've reached the end of the level (camera has fully scrolled to right side of level)
		facing_left = 0;
		gamestate = EXITING_LEVEL_ST;

		return;

	} else if (cam_off < p_x - 127.5) {

		cam_off = p_x - 127.5;
	}

	// running
	if (!input->left && !input->right) {

		p_dx *= 0.95;

	} else if (t_since_grounded != 0 || !input->down) {

		if (input->left) {
			facing_left = 1;
			p_dx -= p_dx > 0 ? PLAYER_RUN_DECEL : PLAYER_RUN_ACCEL;
			if (-p_dx > PLAYER_MAX_SPEED)
				p_dx = -PLAYER_MAX_SPEED;
		}

		if (input->right) {
			facing_left = 0;
			p_dx += p_dx < 0 ? PLAYER_RUN_DECEL : PLAYER_RUN_ACCEL;
			if (p_dx > PLAYER_MAX_SPEED)
				p_dx = PLAYER_MAX_SPEED;
		}

	} else {
		
		p_dx *= 0.95;
	}

	// cache jump
	if (input->action_a_justchanged && input->action_a)
		t_since_jump = 0;

	// jump
	if (t_since_grounded < JUMP_LEEWAY && t_since_jump < JUMP_LEEWAY) {

		p_dy = PLAYER_JUMP_SPEED + (p_dx < 0 ? p_dx : -p_dx) / PLAYER_MAX_SPEED * 0.5;
		t_since_grounded = JUMP_LEEWAY;

		if (!input->action_a) { // the jump was early-released before the jump even started!
			p_dy /= 2;
		}

		t_since_jump = JUMP_LEEWAY; // if we don't do this, then we can actually jump twice for one jump press
	}

	// early release jump
	if (p_dy < 0 && input->action_a_justchanged && !input->action_a) {
		p_dy /= 2;
	}

	// gravity
	p_dy += 0.2;

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
	int anim_x, anim_y;

	if (input->down) {

		anim_x = 0;
		anim_y = 3 * PLAYER_SPRITE_H;

	} else if (t_since_grounded != 0) {

		if (p_dy > -PLAYER_JUMP_SPEED / 2.7) {
			anim_x = 2 * PLAYER_SPRITE_W;
			anim_y = 2 * PLAYER_SPRITE_H;
		} else if (p_dy < PLAYER_JUMP_SPEED / 2.7) {
			anim_x = 0;
			anim_y = 2 * PLAYER_SPRITE_H;
		} else {
			anim_x = PLAYER_SPRITE_W;
			anim_y = 2 * PLAYER_SPRITE_H;
		}
		
	} else if (p_dx == 0 || (!input->left && !input->right && p_dx < PLAYER_RUN_ACCEL && p_dx > -PLAYER_RUN_ACCEL)) {

		// TODO idle animation
		anim_x = 0;
		anim_y = 0;
		p_animt = 0;

	} else if (facing_left == p_dx > 0) { // turning animation occurs when facing left but moving right (or opposite)
	
		anim_x = PLAYER_SPRITE_W;
		anim_y = 3 * PLAYER_SPRITE_H;
		p_animt = 0;
	
	} else {

		anim_x = (int) p_animt % 4 * PLAYER_SPRITE_W;
		anim_y = PLAYER_SPRITE_H;
	}

	draw_level_contents(anim_x, anim_y);

	p_animt += (p_dx > 0 ? p_dx : -p_dx) * 0.1;

	level_animt++;
	t_since_jump++;
	t_since_grounded++;
}

static void update_paused_from_level(const Input *input) {

	draw_level_contents(0, 0);

	draw_text("paused", 111, 8);
}

static void update_ending_level() {

	p_x += PLAYER_MAX_SPEED;

	draw_level_contents((int) p_animt % 4 * PLAYER_SPRITE_W, PLAYER_SPRITE_H);

	p_animt += (p_dx > 0 ? p_dx : -p_dx) * 0.1;

	if (p_x - 64 > LEVEL_WIDTH * 16)
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