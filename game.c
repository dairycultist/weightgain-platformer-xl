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
	{ 128, 32, 32, 0.08, 0.2, 2.5, -5.5 },
	{ 256, 48, 48, 0.035, 0.1, 2.0, -4.0 },
	{ 448, 64, 48, 0.02, 0.05, 1.0, -3.0 },
	{ 640, 96, 48, 0.01, 0.05, 0.5, -1.0 }
};

static int curr_state;

static float px = 40.0, py = 40.0;
static float pdx, pdy;

static float animt;

static int facing_left;
static int grounded;

#define LEVEL_HEIGHT 13

static int level[] = { // to satisfy spatial locality, level data is stored column-major
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
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
};

void game_init() {
	
	set_background(80, 180, 255);
}

void game_update(const Input *input) {

	// draw level
	for (int i = 0; i < sizeof(level) / sizeof(int); i++) {

		if (level[i])
			draw_level(0, (i / LEVEL_HEIGHT) * 16, 72 + (i % LEVEL_HEIGHT) * 16);
	}

	draw_text("vivian.5       world 1-1", 2, 2);

	// temp size-changing
	if (input->action_b_justchanged && input->action_b) {
		curr_state++;
		if (curr_state == sizeof(states) / sizeof(CharacterState))
			curr_state = 0;
	}

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

	// apply velocity
	px += pdx;
	py += pdy;

	// collision
	if (py > 248) {
		py = 248;
		pdy = 1;
		grounded = 1;
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
		
	} else if (!input->left && !input->right && pdx < state.run_acceleration && pdx > -state.run_acceleration) {

		// TODO idle animation
		anim_x = 0;
		anim_y = 0;
		animt = 0;

	} else if (facing_left == pdx > 0) { // turning animation occurs when, say, facing left but moving right
	
		anim_x = state.w;
		anim_y = 3 * state.h;
		animt = 0;
	
	} else {

		anim_x = (int) animt % 4 * state.w;
		anim_y = state.h;
	}

	draw_character(state.w, state.h, anim_x, anim_y + state.ss_off, (int) px - state.w / 2, (int) py - state.h, facing_left);

	animt += (pdx > 0 ? pdx : -pdx) * 0.1;
}