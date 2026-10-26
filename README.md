<p align="center">
  <img src="sce_sys/icon0.png" alt="Left Seat icon" width="128">
</p>

<h1 align="center">Left Seat</h1>

A flight simulator for the PS Vita in the spirit of the early-90s Microsoft Flight Simulator games: flat-shaded
3D, a working instrument panel and a flight model based on a classic four-seat light trainer.

By Kyle Knowles. Written in C with [VitaSDK](https://vitasdk.org) and vita2d. All 3D is done in software
(projection, clipping and sorting) and drawn as triangles. There are no art or audio assets: the world, the
aircraft and every sound are generated in code.

> Left Seat is a hobby project. The aircraft are fictional designs from "Knowles Aircraft Co." and "Ashley Aerospace" and the airports
> and places are made up. It is homebrew and needs a modified Vita.

## Contents

1. [Screenshots](#screenshots)
2. [Install](#install)
3. [Quick start](#quick-start)
4. [Controls](#controls)
5. [Aircraft](#aircraft)
6. [The world](#the-world)
7. [Challenges](#challenges)
8. [Weather, time of day and sound](#weather-time-of-day-and-sound)
9. [Cockpit and cameras](#cockpit-and-cameras)
10. [Building from source](#building-from-source)
11. [Reporting a problem](#reporting-a-problem)
12. [Project layout](#project-layout)
13. [Tuning and roadmap](#tuning-and-roadmap)
14. [License](#license)

## Screenshots

![Chase view of the K-4 on final approach to Knowles Field](docs/images/chase-knowles-field.png)

| | |
|---|---|
| ![K-4 cockpit view on approach](docs/images/cockpit-k4.png)<br>K-4 cockpit: engine cowling, propeller and the instrument panel | ![K-30 cockpit with three green gear lights](docs/images/cockpit-k30.png)<br>K-30 on approach with the gear down: three green gear lights on the panel |
| ![K-10 over the red desert near Redrock](docs/images/redrock-k10.png)<br>K-10 Hauler over the red desert near Redrock | ![Pinecrest valley](docs/images/pinecrest-valley.png)<br>Approaching the Pinecrest valley between two ridges |
| ![Night approach to Knowles Field](docs/images/night-approach.png)<br>Night approach with runway lights and glide-slope lights | ![K-30 Courier from the side](docs/images/k30-side.png)<br>The K-30 Courier in the chase view, gear up |

## Install

**You need** a PS Vita running custom firmware (for example HENkaku), with VitaShell installed. Left Seat is homebrew
and will not install on an unmodified console. It has been tested on a PS Vita 1000-series.

1. Download `leftseat.vpk` from the [Releases](../../releases) page (or from the `leftseat-vpk` artifact on the
   Actions tab, see [Building from source](#building-from-source)).
2. Copy it to your Vita, for example with VitaShell's USB or FTP mode.
3. Open it in VitaShell and install it. Left Seat then appears on the LiveArea screen.

Left Seat saves your best challenge scores and sound volume in `ux0:data/skyhawk_scores.txt`, so updating the game keeps them.

## Quick start

1. Open the **Hangar** and pick an aircraft.
2. Choose the airport, weather and time of day on the start menu.
3. Choose **Open World** to fly freely, or pick a **Challenge** (see below).
4. A typical takeoff in the K-4: flaps 10, full throttle, a little right rudder to hold the centre line, rotate at
   about 55 kt and climb at about 74 kt. Approach at about 65 kt with full flaps and flare just above the runway.

The start menu is a big **OPEN WORLD** button over a grid of tiles: Airport, Hangar, Weather, Time, Challenges, Options, Controls and Quit. On the touch screen, tap a tile. With the buttons, move with the D-pad or left stick and press ✕. Airport, Hangar, Weather and Time open a list with a picture and details on the right: tap a row to look at it, then tap it again or tap SELECT to choose (✕ or SELECT chooses, ○ or BACK cancels). Options is one screen with every setting as a row of buttons: tap a button to set it, or use Up / Down to pick a row and Left / Right (or ✕) to change it. Tap Quit twice (or press START twice) to leave the game.

## Controls

The same list is in the game: choose **CONTROLS** on the start menu.

| Input | Action |
|---|---|
| Left stick | Pitch and roll |
| Right stick up / down | Throttle |
| Right stick left / right | Rudder |
| L / R | Look left / right (in the chase view, both together swing the camera round to the front) |
| D-pad up | Fast forward (free flight only): tap for x2, again for x5, again for normal. Resets on a crash, reset or menu |
| D-pad down | Wheel brakes on the ground. In the air (above about 100 ft): trim for level flight at your current airspeed. Let go of the stick afterwards |
| D-pad left / right | Pitch trim: a tap is a fine step (a tenth of a degree), holding speeds up after a moment |
| ○ / ✕ | Flaps down / up |
| □ | Landing gear up / down (K-30 and Ashley Twin). Refused on the ground and above about 200 kt; takes about 4.5 seconds; landing with it up is a crash |
| △ | Reset the aircraft (restarts the attempt in a challenge) |
| Select (tap) | Switch camera (cockpit / chase) |
| Select (hold) | Chart of the whole area. The flight pauses while it is open. Touch screen: drag to move the map, pinch to zoom, tap an airport to pick it, drag the list to scroll. Buttons: D-pad / left stick move, L / R zoom, X centre on you, Square next airport, O / Select / Start resume |
| L + R + Select | Invert the pitch axis |
| Select + Triangle | Gyro controls: centre the tilt where the Vita is now |
| START | Pause and return to the menu |
| Select + START | Save a screenshot as a PNG in `ux0:data/` (`leftseat_001.png`, `leftseat_002.png`, ...). Copy the files off the Vita with VitaShell |

### Flying by tilt (gyro)

In **Options**, set **Controls** to **GYRO**. Tilt the Vita like a control column: left or right rolls, the top edge towards you is nose up. The tilt is measured from the way you were holding the Vita when the flight started, so any comfortable grip works. SELECT + Triangle centres it again at any time. The tilt adds to the sticks, so you can still use them. The throttle and rudder stay on the right stick.

Options also has **Gyro Roll** and **Gyro Pitch** (reverse a direction) and **Sensitivity** (how far you tilt for full control: about 40, 28 or 18 degrees). The Tilt Test panel on that screen shows the Vita turning with your tilt and two bars for what it asks of the aircraft, so you can check the directions before you fly. The settings are saved.

## Aircraft

Six aircraft: four from Knowles Aircraft Co. and two from Ashley Aerospace, the Ashley Twin and the Ashley Single. Each has its own 3D model, cockpit view, handling, gauges and best
scores. Pick one in the Hangar; it applies to Open World and to every challenge. The Hangar shows a turning 3D picture of the aircraft you are looking at. On the touch screen, tap an aircraft in the list, drag the picture to turn it, and tap SELECT or BACK; the D-pad works too (Up / Down choose, Left / Right turn, X select, O back).

| Aircraft | What it is | Numbers |
|---|---|---|
| **K-4 Trainer** | Slow, forgiving piston four-seater | Stall about 53 kt clean, 48 kt with full flaps |
| **K-10 Hauler** | Big turboprop, about three times the weight | Cruise about 175 kt (Redrock in about 5.5 minutes instead of 9), climb about 970 fpm, stall 56 kt with full flaps, 335 gallons |
| **Jet 1 Sprint** | Tiny single-seat jet: turbojet on top of the rear fuselage, V-tail, bubble canopy | Cruise 210 kt, climb about 1,700 fpm, stall 56 kt (50 kt with flaps). The engine takes a couple of seconds to answer the throttle and the exhaust glows when it works hard |
| **K-30 Courier** | Twin-engine jet with a T-tail, nose canards and retractable gear | Rotates at about 100 kt, cruises about 320 kt, stalls at 82 kt. One throttle drives both engines |
| **Ashley Twin A22** | Six-seat twin turboprop with pusher propellers, twin tail booms, a forward-swept wing, an inverted-V tail and retractable gear | Cruises about 200 kt, stalls about 62 kt with full flaps, 320 gallons. Round passenger windows. One throttle drives both engines |
| **Ashley Single A11** | Four-seat single-engine pusher: low swept wing, twin tail booms, an H-tail and a box-shaped piston engine on top of the rear fuselage | Cruises about 150 kt, stalls about 52 kt with full flaps, 70 gallons. Fixed gear |

The K-30's and the Ashley Twin's gear is raised and lowered with □. Raise it once you are climbing and lower it before you land. Three
gear lights sit on the panel between the engine gauge and the fuel gauge, like a jet's: three green when the gear
is down and locked, red while it moves, dark when it is up.

The flight model covers lift, drag, thrust and stalls tuned toward the published figures of typical aircraft,
flaps in four steps, trim, fuel burn, engine torque and adverse yaw. The wheels are springs and dampers, so
touchdowns are gradual and bounces happen. A bad landing (too hard, sideways, or a wingtip, tail or propeller
strike) is a crash.

## The world

The airports and the country between them. Hold **Select** for a chart of the whole area (the flight pauses; use the touch screen to move and zoom it) showing the airports and the whole coastline (the ocean is always drawn, so follow the coast to find what is out there),
your position and heading, your track so far, and the bearing and distance to each airport.

There is one more place that is not on the chart or in any list until you find it: a long way out, where the farmland ends at an ocean coast. Fly there and it is added to the chart and the start menu.

| Airport | Where | Description |
|---|---|---|
| **Knowles Field (KAK)** | Home | Runway with numbers and aiming-point markings, edge and end lights, glide-slope (PAPI) lights, taxiway and apron, a terminal and hangars |
| **Redrock Municipal (RDR)** | About 16 NM north-north-east of KAK | Runway 07/25, apron, terminal, tower and hangars, and a beacon you can see from far away at night. The farmland turns to red desert with buttes around it |
| **Pinecrest Airstrip (PNC)** | About 11 NM west-north-west of KAK | A 650 m mown-grass strip (runway 12/30, no markings) with a small hangar and cabin, in a forested valley between two ridges. After dark it has low runway lights; from far away you need the chart |
| **Lana Lake Regional (LLK)** | About 8 NM south of KAK | Runway 16/34 with markings, PAPI lights, a parallel taxiway, apron, terminal, tower and three hangars, on the shore of a lake. Edge, threshold and taxiway lights and a beacon after dark |

Between KAK and RDR there is a mountain pass, low hills, towns, a highway, and a river that runs on to a lake at
its mouth. Roads wind through the countryside and some of them cross the river on bridges. The towns have
streets and a central plaza. Elsewhere there are patchwork fields, more lakes with sandy shores and patches of
forest. A windsock by each runway shows the wind.

## Challenges

Pick one from the menu. Each is scored out of 100 on the touchdown: sink rate (40), distance from the centre line
(25), touchdown point (25) and heading and bank (10), minus penalties for bounces and a nose-wheel-first landing.
A crash scores 0. Your best score for each is saved to `ux0:data/skyhawk_scores.txt`. The weather and time of day
chosen on the menu apply to all of them.

| Challenge | What happens |
|---|---|
| **Landing** | You start about 2.4 NM out on final approach, a little off line and height. The chosen weather stays fixed during the attempt |
| **Engine out** | The engine fails at about 1,500 ft, 1.6 to 2 NM from the runway and off to one side. Glide at about 70 kt, use flaps to lose height, and land. The scoring window for the touchdown point is wider |
| **Crosswind** | A strong wind (about 11 kt at the ground, 27 kt higher up, with gusts) blows across the runway from the left or right. Crab into it, then straighten with rudder before touchdown |
| **Cross-country** | Take off from Knowles Field, fly to Redrock and land on its runway from either end. The HUD shows bearing and distance. The result card also shows trip time and fuel used. Landing anywhere else scores 0 |
| **Backcountry** | The same trip, but to Pinecrest: find the grass strip in the mountains and put it down in the first few hundred feet. The scoring area shrinks with the runway, so touching down early matters more |
| **Lake rings** | Not a landing: you start in the air north of Lana Lake Regional and fly through a course of 7 gold rings that bends over and around the lake. 80 points for the rings you fly through, plus up to 20 for beating the par time. A ring you miss is skipped. A crash scores 0. The chart shows the rings still to fly |

## Weather, time of day and sound

**Weather:** choose clear, breezy, overcast, fog, rain or storm and it stays that way for the whole flight.
Choose random and the weather starts anywhere and drifts from one kind to another over time. Wind, gusts and
turbulence affect the aircraft, clouds drift and can be flown through, fog cuts visibility, rain falls with the
wind (straight down when you are parked, slanting back as you fly, sideways past a window) and storms flash with
lightning. The panel shows the weather, wind and whether you are on the ground or airborne.

**Time of day:** day, dusk or night. Dusk has a glowing orange horizon, a low sun and warm light. Night has a
dark sky with a moon and stars, lit airports (runway edge and threshold lights, approach lights, taxiway lights,
tower beacon and windows), glowing glide-slope lights and your landing light.

**Sound:** every sound is made on the fly from what the aircraft is doing, so there are no audio files. There is
a piston engine with a lumpy idle (K-4), a turboprop throb (K-10, Ashley Twin) and a jet whine and roar (Jet 1, K-30), each
following its engine speed; air noise that rises with speed and gusts; a stall warning horn a few knots before the
stall; rain and thunder; tyre chirp and thump, rolling rumble (rougher on grass) and brake squeal; the flap motor;
a sputter when the engine fails; a crash; jingles for good, fair and poor landings; and menu clicks. The cockpit
view is more muffled than the chase view. The SOUND row in the menu sets the volume (OFF, LOW, MEDIUM, HIGH) and
is remembered between sessions. If the sound hardware cannot be opened the game simply runs silent.

## Cockpit and cameras

Two cameras, switched with Select. In the **cockpit** you sit in the left seat with the engine cowling and
propeller blur ahead of you. A windshield and door frame slide across as you look left or right with the
triggers, and the wings, struts and moving flaps and ailerons show out of the side windows. The **chase** view
shows the detailed 3D aircraft with moving control surfaces, flaps and propeller.

The instrument panel has round gauges (airspeed indicator with coloured arcs, attitude indicator, altimeter with
two hands, rotating-card heading indicator, vertical speed, engine speed and fuel), a slip indicator, and
throttle, flap and trim levers.

## Building from source

You need [VitaSDK](https://vitasdk.org/) (with the `vita2d` package), CMake and, optionally, Python 3 (it cleans
the PNG icons before packaging).

```sh
git clone https://github.com/Av8t4/leftseat.git
cd leftseat
mkdir build && cd build
cmake ..
bash ../fixpad.sh      # builds leftseat.vpk; use this instead of plain make
```

The result is `build/leftseat.vpk`.

### Building on GitHub

You do not need VitaSDK on your own machine. Every push builds `leftseat.vpk` with GitHub Actions: open the
repository's **Actions** tab, open the latest run and download the `leftseat-vpk` artifact (a zip containing the
VPK). Pushing a tag such as `v1.0` also attaches the VPK to a Release. See `GITHUB_GUIDE.md` for a step-by-step
walkthrough.

### Troubleshooting

- **`vita-elf-create: Cannot allocate ... bytes for SCE data at end of segment 0; segment 1 overlaps`**
  The converter sometimes needs a little free space after the code. `fixpad.sh` measures it and grows the padding
  array in `src/pad.c` until it fits, then builds. It may edit `src/pad.c`; that is expected.
- **The app icon is white.** Extra metadata blocks inside a PNG can make the Vita's icon loader fail.
  `tools/strip_png.py` removes them, and CMake runs it on the icon and LiveArea images every time it configures.
  By hand: `python3 tools/strip_png.py in.png out.png`.
- **Logs.** The game writes `ux0:data/skyhawk.log` with a few startup lines, a crash reason after a crash, and
  frame timings every 300 frames.

## Reporting a problem

Found a bug? Open an issue on the repository's **Issues** tab and say:

- what you were doing (aircraft, airport, weather, time of day, and where in the world);
- what you expected and what happened;
- your Vita model and firmware version;
- the game version (shown on the release you installed);
- the log file `ux0:data/skyhawk.log`, attached to the issue. It holds a few startup lines, a crash reason and
  frame timings.

## Project layout

`src/main.c` includes the other source files in order, so the game is built as one unit. The files are split by
topic.

```
src/main.c         entry point and main loop; includes the files below
src/defs.inc       constants, types and global state
src/aircraft.inc   every number that makes one aircraft handle differently from another
src/mathx.inc      vector and number helpers
src/batch.inc      collecting triangles and sending them to the GPU
src/weather.inc    weather state, wind, fog and sky colours
src/clip2d.inc     2D polygon clipping and filling
src/camera.inc     3D camera and projection
src/world.inc      static world (buildings, hills, towers, roads, bridges) and collisions
src/flight.inc     aircraft state, control shaping, crash handling
src/gear.inc       landing gear, tyre friction, ground strikes
src/challenge.inc  challenges: starts, scoring and the best-score file
src/update.inc     per-frame flight update (inputs, aerodynamics, ground roll)
src/models.inc     3D K-4 model, cockpit nose, shadow
src/modelkit.inc   kit for smooth models: lofted fuselages, airfoil wings, tail surfaces, tyres
src/k10.inc        3D K-10 model and cockpit parts
src/jet1.inc       3D Jet 1 model and cockpit parts
src/k30.inc        3D K-30 model (twin engines, T-tail, canards, retractable gear) and cockpit parts
src/fleet.inc      chooses the model for the aircraft being flown
src/soundcore.inc  the sound engine: makes every sound on the fly (knows nothing about the Vita)
src/sound.inc      plays it on a background thread and tells it what the aircraft is doing
src/sky.inc        clouds, windsock, rain
src/scenery.inc    fields, lakes, trees, airport detail, cockpit frame
src/night.inc      dusk and night: sky, sun, moon, stars, airport lights, landing light
src/airports.inc   Redrock, the highway, river, country roads, town streets and bridges
src/scene.inc      drawing one frame of the 3D scene
src/panel.inc      instrument panel and gear lights
src/overlays.inc   on-screen messages and warnings
src/gyro.inc       flying by tilt: reads the motion sensor
src/chart.inc      the pausable, touch-screen chart shown when you hold Select
src/spaceport.inc  the east coast sea and the hidden spaceport (discovery, launch site, build site)
src/menu.inc       start menu and the landing result card
src/pad.c          padding used by fixpad.sh
fixpad.sh          build helper
tools/strip_png.py removes extra chunks from PNG files
docs/images/       screenshots used in this README
sce_sys/           icon and LiveArea images and template
CMakeLists.txt     build file
```

## Tuning and roadmap

If the frame rate dips, these are the first things to turn down (near the top of `defs.inc` and `scenery.inc`):

| Setting | Effect |
|---|---|
| `WORLD_DETAIL` (defs.inc) | 2 = everything (default), 1 = no towns, buttes or river, 0 = only the airports and the original mountains |
| `TREE_LIMIT` (scenery.inc) | most trees drawn at once; 0 turns them off |
| `COCKPIT_FRAME` / `COCKPIT_NOSE` (scenery.inc) | the window frame and engine cowling in the cockpit view |
| `MODEL_DETAIL` (k10.inc) | 2 = full K-10 model (default), 1 = lighter, about 550 fewer vertices in the chase view |
| `TILE_R` (defs.inc) | how many ground tiles are drawn around you |

Ideas for later: more aircraft (a row in the table in `aircraft.inc` and a model are all a new one needs), ring
courses, real rolling terrain and more airports.

## License

MIT, see [LICENSE](LICENSE).
