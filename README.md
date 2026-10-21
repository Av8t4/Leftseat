# Left Seat

A flight simulator for the PS Vita, in the spirit of the early-90s Microsoft Flight Simulator games:
flat-shaded 3D, a proper instrument panel and a flight model based on a classic four-seat light trainer.
By Kyle Knowles. Written in C with [VitaSDK](https://vitasdk.org) and vita2d. All 3D is done in software (projection,
clipping and sorting) and drawn as triangles, and there are no art assets to load: the world and the
aircraft are built in code.

> Left Seat is a hobby project. The aircraft in it are fictional designs from "Knowles Aircraft Co.", and the
> airports and places are made up.

## Features

- **Flight model:** lift, drag, thrust and stalls tuned toward the published figures of a typical four-seat light trainer (stall around
  53 kt clean and 48 kt with full flaps), flaps in four steps, trim, fuel burn, engine torque and adverse
  yaw. Rudder rolls the aircraft the way it does in a real one.
- **Landing gear:** spring-and-damper wheels, so touchdowns are gradual, bounces happen, and a bad landing
  (too hard, sideways, or a wingtip, tail or propeller strike) is a crash.
- **Cockpit panel:** seven round gauges (airspeed indicator with coloured arcs, attitude indicator,
  altimeter with two hands, a rotating-card heading indicator, vertical speed, tachometer and fuel gauge),
  a slip indicator, and throttle, flap and trim levers.
- **Cameras:** a cockpit view and a chase view. In the cockpit you sit in the left seat with the engine cowling and
  propeller blur ahead of you. A windshield and door frame slides across as you look left or right with the
  triggers, and the wings, struts and moving flaps and ailerons show out of the side windows. The chase view has a
  detailed 3D K-4 whose control surfaces, flaps and propeller move.
- **Airport:** on the menu, pick where the open world starts (Knowles Field, Redrock or Pinecrest). Triangle resets you to that runway.
- **Weather:** choose clear, breezy, overcast, fog, rain or storm and it stays that way for the whole flight.
  Choose random and the weather starts anywhere and drifts from one kind to another over time. Wind, gusts and
  turbulence affect the aircraft, clouds drift and can be flown through, fog cuts visibility, rain falls
  through the world with the wind (straight down when you are parked, slanting back as you fly, sliding past
  sideways when you look out of a window) and storms flash with lightning. A windsock by the runway shows the wind.
- **Scenery:** patchwork fields, roads, lakes with sandy shores and patches of forest. The airport has a
  taxiway and apron, runway numbers and aiming-point markings, edge and end lights, glide-slope (PAPI) lights
  beside the runway, a terminal and hangars.
- **Sound:** every sound is made on the fly from what the aircraft is doing, so there are no audio files. A piston engine
  with a lumpy idle (K-4), a turboprop with a propeller throb (K-10) and a jet whine and roar (Jet 1), each
  following its own engine speed; air noise that rises with speed and gusts; the stall warning horn a few knots before the
  stall; rain; thunder after lightning; tyre chirp and thump at touchdown, rolling rumble (rougher on grass) and brake squeal;
  the flap motor; a sputter when the engine fails; a crash; jingles for good, fair and poor landings; and menu clicks.
  The cockpit view is more muffled than the chase view, and the tower view fades with distance and shifts pitch as the
  aircraft passes. The SOUND row in the menu sets the volume (OFF, LOW, MEDIUM, HIGH; remembered between sessions).
  If the sound hardware cannot be opened the game simply runs silent.
- **Three airports and cross-country flying:** Redrock Municipal (RDR) is about 29 km north-north-east of
  Knowles Field (KAK), with its own runway 07/25, apron, terminal, tower and hangars, lit at night with a beacon you
  can see from far away. Between the two there is a mountain pass, low hills, towns, a river and a highway; the
  farmland turns to red desert with buttes around Redrock. Hold **Select** for a chart of the whole area with both
  airports, your position and heading, your track so far, and the bearing and distance to each airport.
  The third airport, **Pinecrest Airstrip (PNC)**, is a short mown-grass strip (650 m, runway 12/30, no markings) with a small
  hangar and cabin, about 21 km west-northwest of home. It sits in a forested valley between two mountain ridges,
  with peaks at each end and the approaches kept clear. It has no paint on it, as a grass strip would not: the
  mown stripes, a windsock and, after dark, low runway lights make it findable once you are close; from further out
  you need the chart.
- **Challenges:** pick one from the list in the menu. Each is scored out of 100 on the touchdown: sink rate (40),
  distance from the centre line (25), touchdown point (25) and heading and bank (10), minus penalties for bounces
  and a nose-wheel-first landing. A crash scores 0. Your best score for each is saved to
  `ux0:data/skyhawk_scores.txt`. The time of day you choose applies to all of them.
  - **Landing:** you start about 4.5 km out on final approach, a little off line and height. The weather chosen in
    the menu applies and stays fixed during the attempt.
  - **Engine out:** the engine fails at about 1,500 ft, 3 to 3.7 km from the runway and off to one side. Glide at
    about 70 kt, use flaps to lose height, and land. The scoring window for the touchdown point is wider.
  - **Crosswind:** a strong wind (about 11 kt at the ground, 27 kt higher up, with gusts) blows across the runway
    from the left or the right. Crab into it on the approach, then straighten up with rudder before touchdown.
  - **Cross-country:** take off from Knowles Field, fly to Redrock Municipal and land on its runway, from either end.
    The heads-up display shows the bearing and distance. The score is the same landing score; the result card also
    shows your trip time and fuel used. Landing anywhere else scores 0.
  - **Backcountry:** the same trip, but to Pinecrest: find the grass strip in the mountains and put it down in the
    first few hundred feet. The scoring target area shrinks with the runway, so touching down early matters more.
- **Time of day:** choose day, dusk or night in the menu. Dusk has a glowing orange horizon, a low sun and warm
  light. Night has a dark sky with a moon and stars, a lit airport (runway edge and threshold lights, approach
  lights, taxiway lights, tower beacon and windows), glowing glide-slope lights and your landing light. The
  choice applies to free flight and to the landing challenge, so a night landing is just Challenges at night.
- **Aircraft (Hangar):** four aircraft to fly, all from Knowles Aircraft Co. The **K-4 Trainer** is the slow, forgiving trainer. The
  **K-10 Hauler** is a big turboprop: about three times the weight, cruising at about 175 kt (the trip
  to Redrock takes about five and a half minutes instead of nine), climbing at about 970 fpm, stalling at 56 kt with
  full flaps and carrying 335 gallons. It has its own 3D model, cockpit view, handling, gauges and best scores. Pick
  one in the Hangar; it applies to Open World and to every challenge.
  The **Jet 1 Sprint** is a tiny single-seat jet: a 247 lb turbojet on top of the rear fuselage, a V-tail
  with the exhaust between the two fins, a bubble canopy, a cruise of 210 kt, a climb of about 1,700 fpm and a stall
  at 56 kt (50 kt with flaps). The engine takes a couple of seconds to answer the throttle, the engine gauge reads
  per cent N1 and the exhaust glows when it is working hard.
  The **K-30 Courier** is a twin-engine jet with a T-tail, canards on the nose (only their trailing edges move) and
  retractable landing gear on Square. One throttle drives both engines. It rotates at about 100 kt, cruises at
  about 320 kt and stalls at 82 kt. Raise the gear once you are climbing and lower it before you land; the
  gear indicator is at the top right of the screen.
- **Start menu** with Open World, challenges, the Hangar, weather and time of day.

## Controls

| Input | Action |
|---|---|
| Left stick | Pitch and roll |
| Right stick up / down | Throttle |
| Right stick left / right | Rudder |
| L / R | Look left / right (in the chase view, both together swing the camera round to the front) |
| D-pad up | Fast forward (free flight only, not in challenges): tap for x2, again for x5, again for normal (resets on a crash, reset or menu) |
| D-pad down | Wheel brakes |
| D-pad left / right | Pitch trim |
| ○ / ✕ | Flaps down / up |
| □ | Landing gear up / down (K-30 only). It is refused on the ground and above about 200 kt, takes about 4.5 seconds, and landing with it up is a crash. |
| △ | Reset the aircraft (restarts the attempt in a landing challenge) |
| Select (tap) | Switch camera (cockpit / chase) |
| Select (hold) | Chart of the whole area |
| L + R + Select | Invert the pitch axis |
| START | Pause and return to the menu |

In the menu: D-pad or left stick to move, ✕ to select, left / right on the Weather row to choose the
weather (only Random changes by itself), and START twice to quit.

A typical takeoff: flaps 10, full throttle, a little right rudder to hold the centre line, rotate at about
55 kt and climb at about 74 kt. Approach at about 65 kt with full flaps and flare just above the runway.

## Install

Download `leftseat.vpk` from the [Releases](../../releases) page and install it with VitaShell on a Vita that
runs custom firmware. Left Seat is homebrew and needs a modified console.

## Build it yourself

You need [VitaSDK](https://vitasdk.org/) (with the `vita2d` package), CMake and, optionally, Python 3
(it cleans the PNG icons before packaging).

```sh
git clone https://github.com/<your-username>/leftseat.git
cd leftseat
mkdir build && cd build
cmake ..
bash ../fixpad.sh      # builds leftseat.vpk (see the note below)
```

The result is `build/leftseat.vpk`. Copy it to the Vita and install it with VitaShell.

### Building without a PC (for example from an iPad)

You do not need VitaSDK on your own machine. GitHub can build it for you:

- **Build with GitHub Actions.** Every push builds `leftseat.vpk`. Open the repository's **Actions** tab, open
  the latest run and download the `leftseat-vpk` artifact (a zip containing the VPK).
- **Edit in the browser.** Open the repository and press `.` (or change `github.com` to `github.dev` in the
  address). That opens a lightweight editor; commit your change and Actions does the build.
- **Full terminal in the browser.** On the repository page choose **Code > Codespaces > Create codespace**.
  The `.devcontainer` folder installs VitaSDK for you, so the build commands above work as written.

### Notes and troubleshooting

- **`vita-elf-create: Cannot allocate ... bytes for SCE data at end of segment 0; segment 1 overlaps`**
  The converter sometimes needs a little free space after the code. `fixpad.sh` measures the space and grows
  the padding array in `src/pad.c` until it fits, then builds. Use it instead of plain `make`. It may edit
  `src/pad.c`; that is expected, so commit the change if you like.
- **The app icon is white.** Extra metadata blocks inside a PNG can make the Vita's icon loader fail.
  `tools/strip_png.py` removes them, and CMake runs it on the icon and LiveArea images every time it
  configures. You can also run it by hand: `python3 tools/strip_png.py in.png out.png`.
- **Logs.** The game writes `ux0:data/skyhawk.log` with a few startup lines, and a crash reason after a
  crash.

## Project layout

`src/main.c` includes the other source files in order, so the game is still built as one unit. They are split by topic:

```
src/main.c        entry point and main loop; includes the files below
src/defs.inc      constants, types and global state
src/aircraft.inc  the aircraft: every number that makes one handle differently from another, including jet engine and gauge scales
src/mathx.inc     vector and number helpers
src/batch.inc     collecting triangles and sending them to the GPU
src/weather.inc   weather state, wind, fog and sky colours
src/clip2d.inc    2D polygon clipping and filling
src/camera.inc    3D camera and projection
src/world.inc     static world (buildings, hills, towers) and collisions
src/flight.inc    aircraft state, control shaping, crash handling
src/gear.inc      landing gear, tyre friction, ground strikes
src/challenge.inc challenges: starts, scoring and the best-score file
src/update.inc    per-frame flight update (inputs, aerodynamics, ground roll)
src/models.inc    3D K-4 model, cockpit nose, shadow
src/modelkit.inc   a kit for smooth aircraft models: lofted fuselages, airfoil wings, tail surfaces, tyres
src/k10.inc        3D K-10 model, cockpit parts and shadow
src/jet1.inc       3D Jet 1 model (bubble canopy, V-tail, engine on top), cockpit parts and shadow
src/k30.inc        3D K-30 model (twin engines, T-tail, canards, retractable gear), cockpit parts and shadow
src/fleet.inc     chooses the model for the aircraft being flown
src/soundcore.inc the sound engine: makes every sound on the fly (no audio files), knows nothing about the Vita
src/sound.inc     plays it (a background thread) and tells it what the aircraft is doing each frame
src/sky.inc       clouds, windsock, rain
src/scenery.inc   fields, roads, lakes, trees, airport detail, cockpit frame
src/scene.inc     drawing one frame of the 3D scene
src/panel.inc     instrument panel
src/night.inc     dusk and night: sky, sun, moon, stars, airport lights, landing light
src/airports.inc  Redrock Municipal, the highway and the river (Knowles Field and Pinecrest are in world.inc and scenery.inc)
src/chart.inc     the chart shown while holding Select
src/overlays.inc  on-screen messages and warnings
src/menu.inc      start menu and the landing result card
src/pad.c         padding used by fixpad.sh
fixpad.sh         build helper described above
tools/strip_png.py   removes extra chunks from PNG files
sce_sys/          icon and LiveArea images and template
CMakeLists.txt    build file
```

## Roadmap

Ideas for later: more aircraft (the aircraft table in `aircraft.inc` and a model in `models.inc` are all a new
aircraft needs), ring courses, real rolling terrain and more airports.

## License

MIT, see [LICENSE](LICENSE).

## Tuning

If the frame rate dips, these are the first things to turn down (all near the top of `defs.inc` and `scenery.inc`):

| Setting | Effect |
|---|---|
| `WORLD_DETAIL` (defs.inc) | 2 = everything (default), 1 = no towns, buttes or river, 0 = only the airports and the original mountains. Saves roughly 250 and 900 vertices a frame |
| `TREE_LIMIT` (scenery.inc) | most trees drawn at once; 0 turns them off |
| `COCKPIT_FRAME` / `COCKPIT_NOSE` (scenery.inc) | the window frame and the engine cowling in the cockpit view |
| `MODEL_DETAIL` (k10.inc) | 2 = full K-10 model (default), 1 = lighter: fewer sections and tyre sides, about 550 fewer vertices in the chase view |
| `TILE_R` (defs.inc) | how many ground tiles are drawn around you |
