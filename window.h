#ifndef WINDOW_H
#define WINDOW_H

// screen size
#define WIDTH 256
#define HEIGHT 280
#define ASPECT_RATIO (WIDTH / (float) HEIGHT)

// width and height of sprites
#define SPR_DIM 16

// sprite sheet dimensions in sprites
#define SPRS_WIDTH 16
#define SPRS_HEIGHT 16

typedef struct {

	int up, 		up_justchanged;
	int down, 		down_justchanged;
	int left, 		left_justchanged;
	int right, 		right_justchanged;
	int action_a, 	action_a_justchanged;
	int action_b, 	action_b_justchanged;

} Input;

void draw_sprite(unsigned int sprite, int x, int y, int flip);

void game_init();
void game_update(const Input *input);

#endif