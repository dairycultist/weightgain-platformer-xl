#include "window.h"

static int px = (WIDTH - SPR_DIM) / 2;
static int py = (HEIGHT - SPR_DIM) / 2;

void game_init() {
	
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

	draw_sprite(0, px, py, 0);
}