#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dirent.h>

#include "window.h"

static SDL_Renderer *renderer;
static SDL_Texture *screen_buffer;

static SDL_Texture *ss_level;
static SDL_Texture *ss_font;

static SDL_Texture **ss_characters; // array of SDL_Texture *
static int num_characters;
static int curr_character;

static uint8_t bg_r, bg_g, bg_b;

void set_background(uint8_t r, uint8_t g, uint8_t b) {

	bg_r = r;
	bg_g = g;
	bg_b = b;
}

void draw_character(int w, int h, int from_x, int from_y, int to_x, int to_y, int flip) {

	SDL_Rect copy_rect = { from_x, from_y, w, h };
	SDL_Rect paste_rect = { to_x, to_y, w, h };

	SDL_RenderCopyEx(renderer, ss_characters[curr_character], &copy_rect, &paste_rect, 0.0, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void draw_level(int index, int x, int y) {

	SDL_Rect copy_rect = { (index % 16) * 16, (index / 16) * 16, 16, 16 };
	SDL_Rect paste_rect = { x, y, 16, 16 };

	SDL_RenderCopyEx(renderer, ss_level, &copy_rect, &paste_rect, 0.0, NULL, SDL_FLIP_NONE);
}

void draw_text(const char *string, int x, int y) {

	while (*string != '\0') {

		int i = -1;

		if (*string >= 'a' && *string <= 'z') {

			i = *string - 'a';

		} else if (*string >= '0' && *string <= '9') {

			i = *string - '0' + 26;

		} else if (*string == '-' || *string == '.') {

			i = *string - '-' + 36;
		}

		if (i != -1) {

			SDL_Rect copy_rect = { i * 6, 0, 6, 6 };
			SDL_Rect paste_rect = { x, y, 6, 6 };

			SDL_RenderCopyEx(renderer, ss_font, &copy_rect, &paste_rect, 0.0, NULL, SDL_FLIP_NONE);
		}

		x += 6;

		string++;
	}
}

int main(void) {

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Error initializing SDL:\n%s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);

	if (!window) {
		fprintf(stderr, "Error creating window:\n%s\n", SDL_GetError());
		return 1;
    }

	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!renderer) {
		fprintf(stderr, "Error creating renderer:\n%s\n", SDL_GetError());
		return 1;
	}

	screen_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);

	if (!screen_buffer) {
		fprintf(stderr, "Error creating screen buffer:\n%s\n", SDL_GetError());
		return 1;
	}

	ss_level = IMG_LoadTexture(renderer, "level.png");

	if (!ss_level) {
		fprintf(stderr, "Could not read level.png\n");
		return 1;
	}

	ss_font = IMG_LoadTexture(renderer, "font.png");

	if (!ss_font) {
		fprintf(stderr, "Could not read font.png\n");
		return 1;
	}

	// load all character sprites
	ss_characters = malloc(0);
	num_characters = 0;
	curr_character = 0;

	{
		DIR *dir = opendir("./characters"); 

		if (dir == NULL) {
			fprintf(stderr, "Could not open ./characters folder\n");
			return 1;
		}

		struct dirent *entry;

		while ((entry = readdir(dir)) != NULL) {

			if (strstr(entry->d_name, ".png") != NULL) {

				num_characters++;
				ss_characters = realloc(ss_characters, num_characters * sizeof(SDL_Texture *));

				// if (!ss_characters) {
				// 	allocation failure but idc rn
				// }

				char path[256] = "./characters/";
				strcat(path, entry->d_name);

				ss_characters[num_characters - 1] = IMG_LoadTexture(renderer, path);

				if (!ss_characters[num_characters - 1]) {
					fprintf(stderr, "Could not read ./characters/%s\n", entry->d_name);
					return 1;
				}

				printf("Loaded character: %s\n", entry->d_name);
			}
		}

		closedir(dir);
	}

	// print controls
	printf("\nGood to go!\n\n");
	printf("D-PAD    : Arrow keys\n");
	printf("Action A : Z\n");
	printf("Action B : X\n");

	// init
	game_init();

	// main loop
	SDL_Event event = {0};
	SDL_Rect letterbox = { 0, 0, WIDTH * 2, HEIGHT * 2 };
	Input input = {0};

	while (1) {

		input.up_justchanged       = 0;
		input.down_justchanged     = 0;
		input.left_justchanged     = 0;
		input.right_justchanged    = 0;
		input.action_a_justchanged = 0;
		input.action_b_justchanged = 0;
		
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {

				goto destroy;

			} else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {

				#define MIN(a, b) ((a) > (b) ? (b) : (a))
				#define ASPECT_RATIO (WIDTH / (float) HEIGHT)

				// dynamically change letterbox based on screen resize
				letterbox.w = MIN(event.window.data1, event.window.data2 * ASPECT_RATIO);
				letterbox.h = MIN(event.window.data2, event.window.data1 / ASPECT_RATIO);

				letterbox.x = (event.window.data1 - letterbox.w) / 2;
				letterbox.y = (event.window.data2 - letterbox.h) / 2;

			} else if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && !event.key.repeat) {

				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_UP:
						input.up = event.key.state == SDL_PRESSED;
						input.up_justchanged = 1;
						break;
					case SDL_SCANCODE_DOWN:
						input.down = event.key.state == SDL_PRESSED;
						input.down_justchanged = 1;
						break;
					case SDL_SCANCODE_LEFT:
						input.left = event.key.state == SDL_PRESSED;
						input.left_justchanged = 1;
						break;
					case SDL_SCANCODE_RIGHT:
						input.right = event.key.state == SDL_PRESSED;
						input.right_justchanged = 1;
						break;
					case SDL_SCANCODE_Z:
						input.action_a = event.key.state == SDL_PRESSED;
						input.action_a_justchanged = 1;
						break;
					case SDL_SCANCODE_X:
						input.action_b = event.key.state == SDL_PRESSED;
						input.action_b_justchanged = 1;
						break;
					case SDL_SCANCODE_ESCAPE:
						goto destroy;
						break;
					case SDL_SCANCODE_MINUS: // decrement curr_character
					case SDL_SCANCODE_LEFTBRACKET:
						if (event.type != SDL_KEYDOWN)
							break;
						curr_character--;
						if (curr_character == -1)
							curr_character = num_characters - 1;
						break;
					case SDL_SCANCODE_EQUALS: // increment curr_character
					case SDL_SCANCODE_RIGHTBRACKET:
						if (event.type != SDL_KEYDOWN)
							break;
						curr_character++;
						if (curr_character == num_characters)
							curr_character = 0;
						break;
					default:
						break;
				}
			}
		}

		SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); 			// clear window to grey
		SDL_RenderClear(renderer);
		SDL_SetRenderTarget(renderer, screen_buffer); 				// set render target to screen_buffer
		SDL_SetRenderDrawColor(renderer, bg_r, bg_g, bg_b, 255); 	// clear screen_buffer to black
		SDL_RenderClear(renderer);

		game_update(&input);

		SDL_SetRenderTarget(renderer, NULL); 						// reset render target back to window
		SDL_RenderCopy(renderer, screen_buffer, NULL, &letterbox); 	// render screen_buffer
		SDL_RenderPresent(renderer); 								// present rendered content to screen

		SDL_Delay(1000 / 60);
	}

	// clean up
	destroy:
	SDL_DestroyTexture(ss_level);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}