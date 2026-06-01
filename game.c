#include "window.h"

typedef struct {

	int ss_off; // vertical offset within sprite sheet
	int w, h;

	float run_acceleration;
	float run_deceleration; // higher for quick turning
	float max_run_speed;
	float jump_speed;

} CharacterState;

const static CharacterState states[] = {
	{ 0, 32, 32, 0.1, 0.3, 3.0, -5.5 },
	{ 128, 32, 32, 0.05, 0.15, 2.5, -5.5 },
	{ 256, 48, 48, 0.035, 0.1, 2.0, -4.0 },
	{ 448, 64, 48, 0.02, 0.05, 1.0, -3.0 },
	{ 640, 96, 48, 0.01, 0.05, 0.5, -1.0 }
};

static CharacterState curr_state = states[0];
static int curr_state_i = 0;

static float p_x, p_y;
static float p_dx, p_dy;

static int candy_count;

static int cam_off;

static float p_animt;

static int facing_left;
static int grounded;

#define LEVEL_HEIGHT 13
#define LEVEL_WIDTH (sizeof(level) / sizeof(int) / LEVEL_HEIGHT)

#define IS_COLLIDABLE(i) (i == 1 || i == 2)

static int level_animt;

static int level[] = { // to satisfy spatial locality, level data is stored column-major
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
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
	0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
};

static void get_player_level_aabb(int *start_x, int *start_y, int *end_x, int *end_y) {

	*start_x = (p_x - curr_state.w / 2) / 16;
	*start_y = (p_y - curr_state.h - 72) / 16;

	if (*start_y < 0)
		*start_y = 0;

	*end_x = (p_x - 1 + curr_state.w / 2) / 16;
	*end_y = (p_y - 1 - 72) / 16;

	if (*end_y > LEVEL_HEIGHT - 1)
		*end_y = LEVEL_HEIGHT - 1;
}

static int player_is_colliding() {

	// colliding with left side of screen?
	if (p_x - curr_state.w / 2 < cam_off)
		return 1;

	// TEMP until we have proper level-end, gotta stop the player from clipping into the right edge of the level
	if (p_x + curr_state.w / 2 - 1 > LEVEL_WIDTH * 16)
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

static void increase_state() {

	if (curr_state_i == sizeof(states) / sizeof(CharacterState) - 1)
		return;

	curr_state_i++;

	curr_state = states[curr_state_i];

	// prevent clipping into left side of screen
	if (p_x - curr_state.w / 2 < cam_off)
		p_x = curr_state.w / 2 + cam_off;

	// TEMP until we have proper level-end, gotta stop the player from clipping into the right edge of the level
	if (p_x + curr_state.w / 2 - 1 > LEVEL_WIDTH * 16)
		p_x = LEVEL_WIDTH * 16 - curr_state.w / 2;

	// prevent clipping into a tile
	while (player_is_colliding())
		p_y--;
}

void game_init() {

	p_x = 48.0;
	p_y = 200.0;
	
	set_background(100, 200, 255);
}

void game_update(const Input *input) {

	// update camera offset
	if (p_x + 127.5 > LEVEL_WIDTH * 16) {
		
		cam_off = LEVEL_WIDTH * 16 - WIDTH;

	} else if (cam_off < p_x - 127.5) {

		cam_off = p_x - 127.5;
	}

	// draw level
	for (int i = 0; i < sizeof(level) / sizeof(int); i++) {

		switch (level[i]) {

			default:
			case 0:
				break;
			
			case 1: // unbreakable tile
				draw_level(0, (i / LEVEL_HEIGHT) * 16 - cam_off, 72 + (i % LEVEL_HEIGHT) * 16);
				break;
			
			case 2: // breakable tile
				draw_level(1, (i / LEVEL_HEIGHT) * 16 - cam_off, 72 + (i % LEVEL_HEIGHT) * 16);
				break;
			
			case 3: // candy
				draw_level(2 + ((level_animt / 16) % 4), (i / LEVEL_HEIGHT) * 16 - cam_off, 72 + (i % LEVEL_HEIGHT) * 16);
				break;
		}
	}

	draw_level(2, 2, 2);

	char candy_text[] = ".\0\0\0"; // TODO "world 1-1"
	candy_text[1] = candy_count + '0';
	draw_text(candy_text, 17, 8);

	// running
	if (!input->left && !input->right) {

		p_dx *= 0.95;

	} else if (!grounded || !input->down) {

		if (input->left) {
			facing_left = 1;
			p_dx -= p_dx > 0 ? curr_state.run_deceleration : curr_state.run_acceleration;
			if (-p_dx > curr_state.max_run_speed)
				p_dx = -curr_state.max_run_speed;
		}

		if (input->right) {
			facing_left = 0;
			p_dx += p_dx < 0 ? curr_state.run_deceleration : curr_state.run_acceleration;
			if (p_dx > curr_state.max_run_speed)
				p_dx = curr_state.max_run_speed;
		}

	} else {
		
		p_dx *= 0.95;
	}

	// jump
	if (grounded && input->action_a_justchanged && input->action_a) {
		p_dy = curr_state.jump_speed;
		grounded = 0;
	}

	// early release jump
	if (p_dy < 0 && input->action_a_justchanged && !input->action_a) {
		p_dy /= 2;
	}

	// gravity
	p_dy += 0.2;

	// apply velocity while respecting collision
	p_x += p_dx;

	if (player_is_colliding()) {

		// breakable tile breaking
		if (curr_state_i >= 3) {

			int start_x, start_y, end_x, end_y;
			get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

			int significant_x = p_dx > 0 ? end_x : start_x;
			int broke_something = 0;

			for (int y = start_y; y <= end_y; y++) {

				if (level[y + significant_x * LEVEL_HEIGHT] == 2) {

					level[y + significant_x * LEVEL_HEIGHT] = 0;
					broke_something = 1;
				}
			}

			if (broke_something) {

				p_dx = p_dx < 0 ? 1.0 : -1.0;
				p_dy = -1.5;
				goto skip_horizontal_col;
			}
		}

		// actually do the collision repositioning
		do {
			p_x -= p_dx * 0.1;
		} while (player_is_colliding());

		p_dx = 0;

		skip_horizontal_col:
	}

	p_y += p_dy;

	if (player_is_colliding()) {

		// breakable tile breaking
		if (p_dy < 0 || curr_state_i >= 2) {

			int start_x, start_y, end_x, end_y;
			get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

			int significant_y = p_dy > 0 ? end_y : start_y;
			int broke_something = 0;

			for (int x = start_x; x <= end_x; x++) {

				if (level[significant_y + x * LEVEL_HEIGHT] == 2) {

					level[significant_y + x * LEVEL_HEIGHT] = 0;
					broke_something = 1;
				}
			}

			if (broke_something) {

				p_dy = p_dy < 0 ? 1.0 : -1.5;
				goto skip_vertical_col;
			}
		}

		// actually do the collision repositioning
		do {
			p_y -= p_dy * 0.1;
		} while (player_is_colliding());

		grounded = 1;
		p_dy = 1;

		skip_vertical_col:

	} else {

		grounded = 0;
	}

	// determine animation
	int anim_x, anim_y;

	if (input->down) {

		anim_x = 0;
		anim_y = 3 * curr_state.h;

	} else if (!grounded) {

		if (p_dy > -curr_state.jump_speed / 2.7) {
			anim_x = 2 * curr_state.w;
			anim_y = 2 * curr_state.h;
		} else if (p_dy < curr_state.jump_speed / 2.7) {
			anim_x = 0;
			anim_y = 2 * curr_state.h;
		} else {
			anim_x = curr_state.w;
			anim_y = 2 * curr_state.h;
		}
		
	} else if (p_dx == 0 || (!input->left && !input->right && p_dx < curr_state.run_acceleration && p_dx > -curr_state.run_acceleration)) {

		// TODO idle animation
		anim_x = 0;
		anim_y = 0;
		p_animt = 0;

	} else if (facing_left == p_dx > 0) { // turning animation occurs when facing left but moving right (or opposite)
	
		anim_x = curr_state.w;
		anim_y = 3 * curr_state.h;
		p_animt = 0;
	
	} else {

		anim_x = (int) p_animt % 4 * curr_state.w;
		anim_y = curr_state.h;
	}

	draw_character(curr_state.w, curr_state.h, anim_x, anim_y + curr_state.ss_off, (int) p_x - curr_state.w / 2 - cam_off, (int) p_y - curr_state.h, facing_left);

	p_animt += (p_dx > 0 ? p_dx : -p_dx) * 0.1;

	level_animt++;

	// collect candy
	int start_x, start_y, end_x, end_y;
	get_player_level_aabb(&start_x, &start_y, &end_x, &end_y);

	for (int x = start_x; x <= end_x; x++) {

		for (int y = start_y; y <= end_y; y++) {

			if (level[y + x * LEVEL_HEIGHT] == 3) {

				candy_count++;
				level[y + x * LEVEL_HEIGHT] = 0;

				if (candy_count == 10) {

					candy_count = 0;
					increase_state();
				}
			}
		}
	}
}