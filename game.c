#include "window.h"

typedef struct {

	int ss_off; // vertical offset within sprite sheet
	int w, h;

	// TODO movement parameters

} CharacterState;

static CharacterState states[] = {
	{ 0, 32, 32 },
	{ 96, 32, 32 },
	{ 192, 48, 48 },
	{ 336, 64, 48 }
};

static int curr_state = 0;

static int px = 40;
static int py = 40;

void game_init() {
	
	set_background(80, 180, 255);
}

void game_update(const Input *input) {

	CharacterState state = states[curr_state];

	if (input->up)
		py--;

	if (input->down)
		py++;

	if (input->left)
		px--;

	if (input->right)
		px++;

	if (input->action_a_justchanged && input->action_a) {
		curr_state++;
		if (curr_state == 4)
			curr_state = 0;
	}

	draw_character(state.w, state.h, 0, state.ss_off, px - state.w / 2, py - state.h, 0);
}