#include "window.h"

static int px = 0;
static int py = 0;

void game_init() {
	
	set_background(80, 180, 255);
}

void game_update(const Input *input) {

	if (input->up)
		py--;

	if (input->down)
		py++;

	if (input->left)
		px--;

	if (input->right)
		px++;

	draw_character(16, 16, 0, 0, px, py, 0);
}