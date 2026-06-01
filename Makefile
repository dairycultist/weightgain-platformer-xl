.PHONY: run clean

game.out: *.c ./characters/* ./misc/*
	gcc -o game.out *.c $(shell pkg-config --cflags --libs sdl2 SDL2_image)

run: game.out
	./game.out

clean:
	rm game.out