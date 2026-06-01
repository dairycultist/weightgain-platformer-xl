A simple standalone weight-gain platformer with retexturing support. The weight-gain mechanic plays the role of the
traditional SMB-like powerup system.

Vivian character originally made by (and stolen from) DoubleCap Productions ([source](https://someoneuknow9097.itch.io/smb1r-weight-gain-mod)).

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

Currently you can mod character sprites, level sprites, and the font. (custom levels are planned but not a priority;
for now enjoy my handful of preset levels :3).

The resolution of individual sprites *must* match up and have the same layout. You should also keep the resolution of
the images the same.

Put your character sprites in `./characters/` as a `.png` (I recommend copying an existing one as a template). You can
switch between characters using the -/+ keys.

To mod level sprites, edit the `./misc/level.png` file (currently no switching system).

To mod the font, edit the `./misc/font.png` file (currently no switching system).