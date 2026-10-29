<p align="center">
  <img src="sce_sys/icon0.png" alt="Left Seat icon" width="128">
</p>

<h1 align="center">Left Seat</h1>

A flight simulator for the PS Vita, in the spirit of the early-90s Microsoft Flight Simulator games. It has flat-shaded 3D, a working instrument panel, six aircraft and a small made-up world to fly around.

By Kyle Knowles. Written in C with [VitaSDK](https://vitasdk.org) and vita2d. The 3D is drawn in software, and there are no art or audio files: the world, the aircraft and every sound are generated in code.

> Left Seat is a hobby project. The aircraft are made up ("Knowles Aircraft Co." and "Ashley Aerospace") and so are the airports and places. It is homebrew, so you need a modified Vita.

## Contents

1. [Screenshots](#screenshots)
2. [Install](#install)
3. [Quick start](#quick-start)
4. [Controls](#controls)
5. [Aircraft](#aircraft)
6. [AR view](#ar-view-the-hangar-on-your-desk)
7. [The world](#the-world)
8. [Challenges](#challenges)
9. [Weather, time and sound](#weather-time-and-sound)
10. [Cockpit and cameras](#cockpit-and-cameras)
11. [Building from source](#building-from-source)
12. [Reporting a problem](#reporting-a-problem)
13. [Project layout](#project-layout)
14. [If the frame rate dips](#if-the-frame-rate-dips)
15. [License](#license)

## Screenshots

![Chase view of the K-4 on final approach to Knowles Field](docs/images/chase-knowles-field.png)

| | |
|---|---|
| ![K-4 cockpit view on approach](docs/images/cockpit-k4.png)<br>K-4 cockpit: engine cowling, propeller and instrument panel | ![K-30 cockpit with three green gear lights](docs/images/cockpit-k30.png)<br>K-30 on approach with the gear down: three green lights on the panel |
| ![K-10 over the red desert near Redrock](docs/images/redrock-k10.png)<br>K-10 Hauler over the red desert near Redrock | ![Pinecrest valley](docs/images/pinecrest-valley.png)<br>Approaching the Pinecrest valley between two ridges |
| ![Night approach to Knowles Field](docs/images/night-approach.png)<br>Night approach with runway and glide-slope lights | ![K-30 Courier from the side](docs/images/k30-side.png)<br>The K-30 Courier in the chase view, gear up |

## Install

**You need** a PS Vita running custom firmware (HENkaku, for example) with VitaShell. Left Seat will not install on an unmodified console. It has been tested on a PS Vita 1000.

1. Download `leftseat.vpk` from the [Releases](../../releases) page.
2. Copy it to your Vita (VitaShell's USB or FTP mode works).
3. Open it in VitaShell and install it. Left Seat then appears on the LiveArea screen.

Your best scores and settings are kept in `ux0:data/skyhawk_scores.txt`, so updating the game doesn't lose them.

## Quick start

1. Open the **Hangar** and pick an aircraft.
2. Set the airport, weather and time of day on the start menu.
3. Choose **Open World** to fly freely, or pick a **Challenge**.
4. A typical takeoff in the K-4: flaps 10, full throttle, a little right rudder to hold the center line. Rotate at about 55 kt and climb at about 74 kt. Come in at about 65 kt with full flaps and flare just above the runway.

**Using the menu.** The start menu is a big **Open World** button over a grid of tiles: Airport, Hangar, Weather, Time, Challenges, Options, Controls and Quit.

- **Touch:** tap a tile. In a list, tap a row to look at it, then tap SELECT to choose it or BACK to cancel.
- **Buttons:** move with the D-pad or left stick. ✕ or SELECT chooses, ○ or BACK cancels.
- **Options** is one screen with a row of buttons for every setting. Tap a button, or use Up / Down to pick a row and Left / Right (or ✕) to change it.
- **Quit:** tap it twice, or press START twice.

## Controls

The same list is in the game under **Controls**.

| Input | Action |
|---|---|
| Left stick | Pitch and roll |
| Right stick up / down | Throttle |
| Right stick left / right | Rudder |
| L / R | Look left / right. In the chase view, both together swing the camera round to the front |
| D-pad up | Fast forward in free flight: tap for x2, again for x5, again for normal |
| D-pad down | Wheel brakes on the ground. In the air, trim for level flight at your current speed (let go of the stick afterwards) |
| D-pad left / right | Pitch trim: a tap is a fine step, hold for more |
| ○ / ✕ | Flaps down / up |
| □ | Landing gear up / down (K-30 and Ashley Twin only) |
| △ | Reset the aircraft, or restart the attempt in a challenge |
| Select (tap) | Switch between cockpit and chase camera |
| Select (hold) | Open the chart. The flight pauses while it is open |
| Select + △ | Center the gyro controls |
| L + R + Select | Invert the pitch axis |
| Double-tap the back of the Vita | Swap the gauges for the switch panel, and back (see below) |
| START | Pause and go back to the menu |
| Select + START | Save a screenshot to `ux0:data/` (`leftseat_001.png`, ...) |

**The chart** shows the airports, the coast, your position and track, and the bearing and distance to each airport. Drag to move it, pinch to zoom, tap an airport to pick it. With buttons: D-pad or left stick moves, L / R zoom, ✕ centers on you, □ picks the next airport, ○ / Select / START closes it.

**Landing gear** takes about 4.5 seconds. It won't move on the ground or above about 200 kt, and landing with it up is a crash.

### Flying by tilt (gyro)

In **Options**, set **Controls** to **Gyro**. Tilt the Vita like a control column: left or right rolls, and the top edge toward you is nose up.

- It measures from the way you were holding the Vita when the flight started, so any comfortable grip works. Select + △ centers it again at any time.
- The tilt adds to the sticks, so you can still use them. Throttle and rudder stay on the right stick.
- Options also has **Gyro Roll** and **Gyro Pitch** to reverse a direction, and **Sensitivity** (how far you tilt for full control: about 40, 28 or 18 degrees).
- The Tilt Test panel on that screen shows the Vita turning with you, so you can check the directions before you fly.

## Aircraft

Six aircraft: four from Knowles Aircraft Co. and two from Ashley Aerospace. Each has its own 3D model, cockpit, handling, gauges and best scores. Pick one in the Hangar and it applies to Open World and every challenge.

| Aircraft | What it is | Numbers |
|---|---|---|
| **K-4 Trainer** | Slow, forgiving piston four-seater | Stalls at about 53 kt clean, 48 kt with full flaps |
| **K-10 Hauler** | Big turboprop, about three times the weight | Cruises at about 175 kt, climbs about 970 fpm, stalls at 56 kt with full flaps, carries 335 gallons |
| **Jet 1 Sprint** | Tiny single-seat jet with a V-tail and a bubble canopy | Cruises at 210 kt, climbs about 1,700 fpm, stalls at 56 kt (50 with flaps). The engine is slow to answer the throttle and the exhaust glows when it works hard |
| **K-30 Courier** | Twin jet with a T-tail, nose canards and retractable gear | Rotates at about 100 kt, cruises at about 320 kt, stalls at 82 kt. One throttle drives both engines |
| **Ashley Twin A22** | Six-seat twin turboprop with pusher props, twin booms, a forward-swept wing, an inverted-V tail and retractable gear | Cruises at about 200 kt, stalls at about 62 kt with full flaps, carries 320 gallons. One throttle drives both engines |
| **Ashley Single A11** | Four-seat single pusher with a swept wing, twin booms and an H-tail | Cruises at about 150 kt, stalls at about 52 kt with full flaps, carries 70 gallons. Fixed gear |

**Gear lights.** The K-30 and Ashley Twin have three gear lights on the panel, like a jet: three green when the gear is down and locked, red while it moves, dark when it is up. Raise the gear once you're climbing and lower it before you land.

**Flight model.** Lift, drag, thrust and stalls are tuned toward the published figures of typical aircraft. There are four flap steps, trim, fuel burn, engine torque and adverse yaw. The wheels are springs and dampers, so touchdowns are gradual and bounces happen. A hard or sideways landing, or hitting a wingtip, tail or propeller, is a crash.

## AR view (the Hangar on your desk)

The Hangar shows a turning 3D picture of the aircraft you're looking at. For a closer look, put it on your desk.

1. In the Hangar, press **Square** or tap **AR VIEW**. The Vita's back camera switches on.
2. Lay a card with a thick black square border flat on your desk. Use one of the Vita's own AR cards, or print `docs/ar_marker_a4.png` at 100%.
3. Point the camera at it from about 25 cm, with the whole black border in view. The aircraft stands on the card.
4. Move the Vita around to look at it from any side.

| Input | Action |
|---|---|
| Up / Down | Change aircraft |
| Left / Right | Turn the aircraft on the card |
| L / R | Make it smaller / bigger |
| Triangle | Change the perspective (try this if the aircraft seems to slide when you move the Vita) |
| ✕ | Pick this aircraft for flying |
| ○ | Back to the Hangar |

The aircraft is sized to the card, so a bigger card gives a bigger aircraft. Its nose points toward the top edge of the card. For the steadiest picture, keep the card flat, matt and well lit, and hold the Vita still.

## The world

There are four airports, and one more place you have to find. Hold **Select** for the chart of the whole area. The flight pauses while it is open.

| Airport | Where | Description |
|---|---|---|
| **Knowles Field (KAK)** | Home | Runway with numbers and aiming-point markings, edge and end lights, glide-slope (PAPI) lights, taxiway, apron, terminal and hangars |
| **Redrock Municipal (RDR)** | About 16 NM north-north-east of KAK | Runway 07/25 on an apron with a terminal, tower and hangars, and a beacon you can see from far away at night. The farmland turns to red desert with buttes around it |
| **Pinecrest Airstrip (PNC)** | About 11 NM west-north-west of KAK | A 650 m mown-grass strip (runway 12/30, no markings) with a small hangar and cabin, in a forested valley between two ridges. After dark it has low runway lights. From far away you need the chart |
| **Lana Lake Regional (LLK)** | About 8 NM south of KAK | Runway 16/34 with markings, PAPI lights, a parallel taxiway, apron, terminal, tower and three hangars, on the shore of a lake. Edge, threshold and taxiway lights and a beacon after dark |

**Around Knowles Field.** Downtown Knowles is a real city: houses thin out into streets and blocks, then into a tower district with rooftop lights at night (the red beacons all flash on their own timing). East of the airport is Knowles Works, a giant factory with a village beside it.

**The countryside.** Patchwork fields, forests, lakes with sandy shores and red desert in the north. Mountains rise between the airports, with snow on the tallest. A river runs from the hills to a lake, with roads, a highway and bridges crossing it. Towns have streets, a plaza, water towers, churches and silos, and their windows light up at night. A windsock by each runway shows the wind.

**The coast.** Far to the east the farmland ends at the sea, with beaches, waves, sea stacks and islands. The ocean is always drawn, so follow the coast on the chart to see what's out there.

**The hidden place.** One more place isn't on the chart or in any list. Fly to the coast and look. Once you're close, it's added to the chart and the start menu.

## Challenges

Pick one from the menu. Each is scored out of 100 on the touchdown:

- sink rate: 40
- distance from the center line: 25
- touchdown point: 25
- heading and bank: 10

Bounces and a nose-wheel-first landing cost points, and a crash scores 0. Your best score for each is saved. The weather and time of day you picked apply to all of them.

| Challenge | What happens |
|---|---|
| **Landing** | You start about 2.4 NM out on final approach, a little off line and height. The weather stays fixed during the attempt |
| **Engine out** | The engine fails at about 1,500 ft, 1.6 to 2 NM from the runway and off to one side. Glide at about 70 kt, use flaps to lose height, and land. The touchdown window is wider |
| **Crosswind** | A strong wind (about 11 kt at the ground, 27 kt higher up, with gusts) blows across the runway. Crab into it, then straighten with rudder before touchdown |
| **Cross-country** | Take off from Knowles Field, fly to Redrock and land from either end. The HUD shows bearing and distance, and the result card shows trip time and fuel used. Landing anywhere else scores 0 |
| **Backcountry** | The same trip to Pinecrest. Find the grass strip and put it down in the first few hundred feet. The scoring area is smaller, so touching down early matters more |
| **Lake rings** | You start in the air north of Lana Lake and fly through 7 gold rings that bend over and around the lake. 80 points for the rings, plus up to 20 for beating the par time. A missed ring is skipped, and a crash scores 0 |

## Weather, time and sound

**Weather** can be clear, breezy, overcast, fog, rain, storm or random. Random starts anywhere and drifts from one kind to another. Wind, gusts and turbulence move the aircraft, clouds drift and can be flown through, and fog cuts visibility. Rain falls with the wind, and storms flash with lightning.

**Time of day** can be day, dusk or night. Dusk has a glowing orange horizon and a low sun. Night has a dark sky with a moon and stars, lit airports, glowing glide-slope lights, lit towns and a landing light. Every aircraft carries navigation lights (red left, green right, white tail), a flashing red beacon and white wingtip strobes, which you can see in the chase view. They are switched on the switch panel.

**Sound** is made on the fly from what the aircraft is doing, so there are no audio files:

- a lumpy piston idle (K-4), a turboprop throb (K-10, Ashley Twin) and a jet whine and roar (Jet 1, K-30)
- wind noise that rises with speed, and a stall horn a few knots before the stall
- rain and thunder
- tire chirp, rolling rumble (rougher on grass) and brake squeal
- the flap motor, an engine sputter, a crash, jingles for good, fair and poor landings, and menu clicks

The cockpit is more muffled than the chase view. Volume is in Options (Off, Low, Medium, High) and is remembered. If the sound hardware can't be opened, the game runs silent.

## Cockpit and cameras

Select switches between two cameras. In the **cockpit** you sit in the left seat with the engine cowling and propeller blur ahead of you. The windshield and door frame slide across as you look left and right with L / R, and the wings, struts and moving flaps and ailerons show out of the side windows. The **chase** view shows the detailed 3D aircraft with moving control surfaces, flaps and propeller.

The instrument panel has round gauges (airspeed with colored arcs, attitude indicator, altimeter, heading, vertical speed, engine speed and fuel), a slip indicator, and throttle, flap and trim levers.

**The switch panel.** Double-tap the back of the Vita and the gauges give way to the cockpit switches. Double-tap again to bring the gauges back. Tap a switch on the touch screen to flip it. The panel has the engine switches (battery, fuel, ignition and a START button), the aircraft lights (beacon, navigation, strobes, landing and taxi) and a few extra switches that do nothing yet (pitot heat, avionics, cabin). The QUICK START button turns everything on and starts the engine at once.

**Starting the engine.** Every flight on the ground begins cold and dark: all the switches are off and the engine is stopped. To start it, turn on the battery, the fuel pump and the magnetos, then hold the START button until the engine catches. The turboprops and jets work the same way with fuel and ignition, but they take a few seconds longer to light and spool up. Turn off the magnetos (piston) or the fuel (turbine) to shut the engine down. The lights need the battery on. Challenges start in the air with the engine running and the lights on. Your switches stay as you left them when you return from the menu, and a reset (△), a new aircraft or a new airport puts them back to cold and dark.

If your Vita's back pad is not responding, or the double tap is too fussy, two quick taps on the instrument panel with the touch screen do the same thing. You can change how fast the two taps must follow each other (or turn the back pad off) in **Options**, under Switch Panel.

## Building from source

You need [VitaSDK](https://vitasdk.org/) with the `vita2d` package, and CMake. Python 3 is optional (it cleans the icons before packaging).

```sh
git clone https://github.com/Av8t4/leftseat.git
cd leftseat
mkdir build && cd build
cmake ..
bash ../fixpad.sh      # builds leftseat.vpk; use this instead of plain make
```

The result is `build/leftseat.vpk`.

### Building on GitHub

You don't need VitaSDK on your own machine. Every push builds `leftseat.vpk` with GitHub Actions: open the **Actions** tab, open the latest run and download the `leftseat-vpk` artifact (a zip containing the VPK). Pushing a tag such as `v2.1` also attaches the VPK to a Release. `GITHUB_GUIDE.md` has a step-by-step walkthrough.

### Troubleshooting

- **`Cannot allocate ... bytes for SCE data at end of segment 0`.** The converter sometimes needs a little free space after the code. `fixpad.sh` grows the padding in `src/pad.c` until it fits, then builds. It may edit `src/pad.c`, and that's expected.
- **The app icon is white.** Extra metadata blocks inside a PNG can make the Vita's icon loader fail. `tools/strip_png.py` removes them, and CMake runs it every time it configures. By hand: `python3 tools/strip_png.py in.png out.png`.
- **Version error.** `VITA_VERSION` in `CMakeLists.txt` must look like `02.10` (two digits, a dot, two digits).
- **Logs.** The game writes `ux0:data/skyhawk.log` with a few startup lines, a crash reason after a crash, and frame timings every 300 frames.

## Reporting a problem

Open an issue on the **Issues** tab and say:

- what you were doing (aircraft, airport, weather, time of day, and where in the world)
- what you expected and what happened
- your Vita model and firmware version
- the game version (shown on the release you installed)
- the log file `ux0:data/skyhawk.log`, attached

## Project layout

`src/main.c` includes the other source files in order, so the game is built as one unit. The files are split by topic.

```
src/main.c         entry point and main loop; includes the files below
src/defs.inc       constants, types and global state
src/aircraft.inc   every number that makes one aircraft handle differently from another
src/mathx.inc      vector and number helpers
src/batch.inc      collecting triangles and sending them to the GPU
src/weather.inc    weather state, wind, fog and sky colors
src/clip2d.inc     2D polygon clipping and filling
src/camera.inc     3D camera and projection
src/world.inc      static world (towns, downtown, factory, mountains, coast) and collisions
src/flight.inc     aircraft state, control shaping, crash handling
src/gear.inc       landing gear, tire friction, ground strikes
src/challenge.inc  challenges: starts, scoring and the best-score file
src/update.inc     per-frame flight update (inputs, aerodynamics, ground roll)
src/models.inc     3D K-4 model, cockpit nose, shadow
src/modelkit.inc   kit for smooth models: lofted fuselages, airfoil wings, tail surfaces, tires
src/k10.inc, jet1.inc, k30.inc, a22.inc, a11.inc   the other aircraft models and cockpit parts
src/fleet.inc      chooses the model for the aircraft being flown
src/soundcore.inc  the sound engine: makes every sound on the fly (knows nothing about the Vita)
src/sound.inc      plays it on a background thread and tells it what the aircraft is doing
src/sky.inc        clouds, windsock, rain
src/scenery.inc    fields, biomes, lakes, trees, airport detail, cockpit frame
src/night.inc      dusk and night: sky, sun, moon, stars, airport lights, landing light
src/airports.inc   airports, highway, roads, river, bridges, town and downtown streets
src/spaceport.inc  the east coast and sea, and the hidden place
src/scene.inc      drawing one frame of the 3D scene, including sorted night lights
src/switches.inc   the engine start-up and the touch handling for the switch panel
src/panel.inc      instrument panel, switch panel and gear lights
src/overlays.inc   on-screen messages and warnings
src/gyro.inc       flying by tilt: reads the motion sensor
src/chart.inc      the pausable, touch-screen chart shown when you hold Select
src/menu.inc       start menu, Hangar and the landing result card
src/ardetect.inc   finds the AR card in a camera picture (no Vita calls, so it can be tested on a PC)
src/ar.inc         the AR view: camera, tracking and drawing the aircraft on the card
src/screenshot.inc saving screenshots
src/pad.c          padding used by fixpad.sh
fixpad.sh          build helper
tools/             strip_png.py (cleans PNGs) and make_ar_marker.py (makes the printable AR card)
docs/              screenshots and the printable AR card
sce_sys/           icon and LiveArea images and template
CMakeLists.txt     build file
```

## If the frame rate dips

These are the first things to turn down:

| Setting | Effect |
|---|---|
| `CITY_DETAIL` (defs.inc) | Downtown Knowles: 2 = full city (default), 1 = about half the buildings, 0 = none |
| `WORLD_DETAIL` (defs.inc) | 2 = everything (default), 1 = no towns, buttes or river, 0 = only the airports and the original mountains |
| `TILE_R` (defs.inc) | How many ground tiles are drawn around you |
| `TREE_LIMIT` (scenery.inc) | Most trees drawn at once. 0 turns them off |
| `COCKPIT_FRAME` / `COCKPIT_NOSE` (scenery.inc) | The window frame and engine cowling in the cockpit view |
| `MODEL_DETAIL` (k10.inc) | 2 = full K-10 model (default), 1 = lighter |

Ideas for later: more aircraft (a row in `aircraft.inc` and a model are all a new one needs), more airports and more places to explore.

## License

MIT, see [LICENSE](LICENSE).
