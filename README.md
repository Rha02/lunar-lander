# Lunar Lander Simulation
## Installation
- Generate a project with OpenFrameworks Project generator.
- Add `ofxAssimpModelLoader` add-on to the project.
- Copy all the source code into the project's `src` folder.
- Copy the `data` folder into the `bin/data` folder.
    - New/custom models are: `ufo_lander.fbx` and `terrain.fbx`.

## How to play
### Game Environment
To switch between game environments, change the `gameEnv` private variable inside `App.h` file.
```cpp
...
	void setupLander();

	GameEnv gameEnv = MOON; // Change game environment
public:
	void setup();
...
```

The available game environments are:
\
`MOON` - uses default moon terrain and the lunar lander model.
\
`DESERT` - uses custom desert terrain and a custom ufo lander.

### Game states
`PREGAME` - Game has not started yet.
- Player can click and drag lander across the terrain.
- Player can press `SPACEBAR` to start the game.

`INGAME` - Game has started.
- Player can move lander via Lander's controls.
- Physics system and motion is applied on the lander and particles.

`ENDGAME` - Game has ended by player either winning or losing.
- Player cannot move the lander at all.
- Physics and Particle system is still enabled.
- Player can press `P` to play again. This will move back to `PREGAME` state.

### Lander Controls
`W` - move lander up.
\
`S` - move lander down.
\
`A` - counter-clockwise rotate.
\
`D` - clockwise rotate.
\
`UP_ARROW` - move forward.
\
`DOWN_ARROW` - move backwawrd.
\
`LEFT_ARROW` - move to the left side.
\
`RIGHT_ARROW` - move to the right side.

### Camera Controls
`F1` - use free movement camera.
- Press `C` to toggle camera movement.
- Camera can be moved via mouse dragging, scrolling, and middle-mouse dragging.
- Press `R` to re-center camera on the lander.

`F2` - use tracking camera.
\
`F3` - use lander's first-person/on-board camera.

### Other controls
`H` - toggle displaying AGL.

### Game Rules
- Player must successfully (slowly) land on all three landing areas that are illuminated by light-blue lights to win.
    - Player receives 10 points for successfully landing at the center of the landing area.
    - A landed area will be illuminated by a green light.
- Player has 2 minutes worth of fuel. If the fuel runs out, the player loses.
- If the player crash-lands (lands too fast) into the terrain, the ship explodes and the player loses.