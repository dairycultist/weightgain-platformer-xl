simple standalone weightgain platformer with preset levels (like, 4 or something; tileset is `level.png`) and moddable character sprites (`player.png`)

weightgain mechanic: bar at the top shows how many strawberries you have to eat before going up a size, getting hit reduces you by a size

level modding MAYBE but not priority

## Installation

1. Install dependencies.

On **Mac**:

```
brew install sdl2
brew install sdl2_image
```

On **Ubuntu/Debian**:

```
sudo apt update
sudo apt install libsdl2-dev
```

2. Then clone, build, and run the project.

```
git clone https://github.com/dairycultist/weightgain-platformer-xl
cd weightgain-platformer-xl
make run
```