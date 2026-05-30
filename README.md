simple standalone weightgain platformer with preset levels (like, 4 or something)

weightgain mechanic: bar at the top shows how many candies you have to eat before going up a size, getting hit reduces you by a size

## Installation

1. Install dependencies. Windows isn't supported yet because of `<dirent.h>` for directory traversal (windows uses `<windows.h>`).

On **Mac**:

```
brew install sdl2
brew install sdl2_image
```

On **Ubuntu/Debian**:

```
sudo apt update
sudo apt install libsdl2-dev
sudo apt install libsdl2-image-dev
```

2. Clone, build, and run the project.

```
git clone https://github.com/dairycultist/weightgain-platformer-xl
cd weightgain-platformer-xl
make run
```

## Modding

Currently you can only mod character sprites (might add level sprites and level layout modding as well not a priority tho)

Put your sprites in `./characters/` as a `.png`.