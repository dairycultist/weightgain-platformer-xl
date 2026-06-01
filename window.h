#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

// screen size
#define WIDTH 256
#define HEIGHT 280

typedef struct {

	int up, 		up_justchanged;
	int down, 		down_justchanged;
	int left, 		left_justchanged;
	int right, 		right_justchanged;
	int action_a, 	action_a_justchanged;
	int action_b, 	action_b_justchanged;
	int pause,      pause_justchanged;

} Input;

void draw_character(int w, int h, int from_x, int from_y, int to_x, int to_y, int flip);
void draw_level(int index, int x, int y);
void draw_level_flip(int index, int x, int y, int flip);
void draw_text(const char *string, int x, int y);
void set_background(uint8_t r, uint8_t g, uint8_t b);

void game_init();
void game_update(const Input *input);

#endif