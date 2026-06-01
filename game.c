#include "window.h"

typedef struct {

	int ss_off; // vertical offset within sprite sheet
	int w, h;

	float run_acceleration;
	float run_deceleration; // higher for quick turning
	float max_run_speed;
	float jump_speed;

} CharacterState;

static CharacterState states[] = {
	{ 0, 32, 32, 0.1, 0.3, 3.0, -5.5 },
	{ 128, 32, 32, 0.05, 0.15, 2.5, -5.5 },
	{ 256, 48, 48, 0.035, 0.1, 2.0, -4.0 },
	{ 448, 64, 48, 0.02, 0.05, 1.0, -3.0 },
	{ 640, 96, 48, 0.01, 0.05, 0.5, -1.0 }
};

static int curr_state;

static float px, py;
static float pdx, pdy;

static int cam_off;

static float animt;

static int facing_left;
static int grounded;

#define LEVEL_HEIGHT 13
#define LEVEL_WIDTH (sizeof(level) / sizeof(int) / LEVEL_HEIGHT)

static int level[] = { // to satisfy spatial locality, level data is stored column-major
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
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
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
};

static int player_is_colliding() {

	CharacterState state = states[curr_state];

	// with left side of screen
	if (px - state.w / 2 < cam_off)
		return 1;

	// with the level
	int start_x = (px - state.w / 2) / 16;
	int start_y = (py - state.h - 72) / 16;

	if (start_y < 0)
		start_y = 0;

	for (int x = start_x; x < start_x + (state.w / 16); x++) {

		for (int y = start_y; y < start_y + (state.h / 16); y++) {

			if (level[y + x * LEVEL_HEIGHT])
				return 1;
		}
	}

	// far positive edges is unique because math
	int farstart_x = (px - 1 - state.w / 2) / 16;
	int farstart_y = (py - 1 - state.h - 72) / 16;

	if (farstart_y < 0)
		farstart_y = 0;

	for (int x = start_x; x < start_x + (state.w / 16); x++) {

		if (level[farstart_y + (state.h / 16) + x * LEVEL_HEIGHT])
			return 1;
	}

	for (int y = start_y; y < start_y + (state.h / 16); y++) {

		if (level[y + (farstart_x + (state.w / 16)) * LEVEL_HEIGHT])
			return 1;
	}

	// and the far positive corner
	if (level[farstart_y + (state.h / 16) + (farstart_x + (state.w / 16)) * LEVEL_HEIGHT])
		return 1;

	return 0;
}

static void increase_state() {

	curr_state++;
	
	if (curr_state == sizeof(states) / sizeof(CharacterState))
		curr_state = 0;

	CharacterState state = states[curr_state];

	// prevent clipping into left side of screen
	if (px - state.w / 2 < cam_off)
		px = state.w / 2 + cam_off;

	// prevent clipping into a tile
	while (player_is_colliding())
		py--;
}

void game_init() {

	px = 48.0;
	py = 248.0;
	
	set_background(80, 180, 255);
}

void game_update(const Input *input) {

	// update camera offset
	if (px + 128 > LEVEL_WIDTH * 16) {
		
		cam_off = LEVEL_WIDTH * 16 - WIDTH;

	} else if (cam_off < px - 128) {

		cam_off = px - 128;
	}

	// draw level
	for (int i = 0; i < sizeof(level) / sizeof(int); i++) {

		if (level[i])
			draw_level(0, (i / LEVEL_HEIGHT) * 16 - cam_off, 72 + (i % LEVEL_HEIGHT) * 16);
	}

	draw_text("vivian.5       world 1-1", 2, 2);

	// temp size-changing
	if (input->action_b_justchanged && input->action_b)
		increase_state();

	CharacterState state = states[curr_state];

	// running
	if (!input->left && !input->right) {

		pdx *= 0.95;

	} else if (!grounded || !input->down) {

		if (input->left) {
			facing_left = 1;
			pdx -= pdx > 0 ? state.run_deceleration : state.run_acceleration;
			if (-pdx > state.max_run_speed)
				pdx = -state.max_run_speed;
		}

		if (input->right) {
			facing_left = 0;
			pdx += pdx < 0 ? state.run_deceleration : state.run_acceleration;
			if (pdx > state.max_run_speed)
				pdx = state.max_run_speed;
		}

	} else {
		
		pdx *= 0.95;
	}

	// jump
	if (grounded && input->action_a_justchanged && input->action_a) {
		pdy = state.jump_speed;
		grounded = 0;
	}

	// early release jump
	if (pdy < 0 && input->action_a_justchanged && !input->action_a) {
		pdy /= 2;
	}

	// gravity
	pdy += 0.2;

	// apply velocity while respecting collision
	px += pdx;

	if (player_is_colliding()) {

		do {
			px -= pdx * 0.1;
		} while (player_is_colliding());

		pdx = 0;
	}

	py += pdy;

	if (player_is_colliding()) {

		do {
			py -= pdy * 0.1;
		} while (player_is_colliding());

		grounded = 1;
		pdy = 1;

	} else {

		grounded = 0;
	}

	// determine animation
	int anim_x, anim_y;

	if (input->down) {

		anim_x = 0;
		anim_y = 3 * state.h;

	} else if (!grounded) {

		if (pdy > -state.jump_speed / 2.7) {
			anim_x = 2 * state.w;
			anim_y = 2 * state.h;
		} else if (pdy < state.jump_speed / 2.7) {
			anim_x = 0;
			anim_y = 2 * state.h;
		} else {
			anim_x = state.w;
			anim_y = 2 * state.h;
		}
		
	} else if (pdx == 0 || (!input->left && !input->right && pdx < state.run_acceleration && pdx > -state.run_acceleration)) {

		// TODO idle animation
		anim_x = 0;
		anim_y = 0;
		animt = 0;

	} else if (facing_left == pdx > 0) { // turning animation occurs when facing left but moving right (or opposite)
	
		anim_x = state.w;
		anim_y = 3 * state.h;
		animt = 0;
	
	} else {

		anim_x = (int) animt % 4 * state.w;
		anim_y = state.h;
	}

	draw_character(state.w, state.h, anim_x, anim_y + state.ss_off, (int) px - state.w / 2 - cam_off, (int) py - state.h, facing_left);

	animt += (pdx > 0 ? pdx : -pdx) * 0.1;
}