#ifndef WINDOW_H
#define WINDOW_H

// screen size
#define WIDTH 256
#define HEIGHT 280
#define ASPECT_RATIO (WIDTH / (float) HEIGHT)

typedef struct {

	int up, 		up_justchanged;
	int down, 		down_justchanged;
	int left, 		left_justchanged;
	int right, 		right_justchanged;
	int action_a, 	action_a_justchanged;
	int action_b, 	action_b_justchanged;

} Input;

void draw_sprite(int w, int h, int from_x, int from_y, int to_x, int to_y, int flip);

void game_init();
void game_update(const Input *input);

#endif