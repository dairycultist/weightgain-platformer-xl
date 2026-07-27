playing a two-player platformer with a controller is so fun

maybe with some undertale dialogue boxes idk

P1 P2

picking up and throwing things is a primary mechanic

Vivian character originally made by (and stolen from) DoubleCap Productions ([source](https://someoneuknow9097.itch.io/smb1r-weight-gain-mod)).

## Installation

1. Install dependencies. Windows isn't supported yet because I don't have a Windows machine to test on (and because Windows traverses directories differently than Mac/Linux which I'd have to implement).

On **Ubuntu/Debian**:

```
sudo apt update
sudo apt install libsdl2-dev
sudo apt install libsdl2-image-dev
```

On **Mac** (I haven't actually tested compiling it on Mac, the gcc flags might be wrong):

```
brew install sdl2
brew install sdl2_image
```

2. Clone, build, and run the project.

```
git clone https://github.com/dairycultist/weightgain-platformer-xl
cd weightgain-platformer-xl
make run
```

## Custom characters

Put your character sprites in `./characters/` as a `.png`. The resolution of individual sprites *must* match up and have the same layout (I recommend copying an existing one as a template). You should also keep the resolution of the images the same. You can switch between characters using the -/+ keys.

## Custom levels

planned