<p align="center">
  <img src="sce_sys/icon0.png" alt="Left Seat icon" width="128">
</p>

<h1 align="center">Left Seat</h1>

A flight simulator for the PS Vita, in the spirit of the early-90s Microsoft Flight Simulator games. It has flat-shaded 3D, a working instrument panel, and six aircraft to fly around a small made-up world.

I'm Kyle Knowles and this is a hobby project. It's written in C with [VitaSDK](https://vitasdk.org) and vita2d. The 3D is drawn in software, and there are no art or audio files: the world, the aircraft and every sound are generated in code. The aircraft are made up ("Knowles Aircraft Co." and "Ashley Aerospace") and so are the airports. It's homebrew, so you need a modified Vita.

![Chase view of the K-4 on final approach to Knowles Field](docs/images/chase-knowles-field.png)

| | |
|---|---|
| ![K-4 cockpit view on approach](docs/images/cockpit-k4.png)<br>K-4 cockpit | ![K-30 cockpit with three green gear lights](docs/images/cockpit-k30.png)<br>K-30 on approach with the gear down |
| ![K-10 over the red desert near Redrock](docs/images/redrock-k10.png)<br>K-10 Hauler near Redrock | ![Pinecrest valley](docs/images/pinecrest-valley.png)<br>The Pinecrest valley |
| ![Night approach to Knowles Field](docs/images/night-approach.png)<br>Night approach to Knowles Field | ![K-30 Courier from the side](docs/images/k30-side.png)<br>K-30 Courier, gear up |

## Install

You need a Vita running custom firmware (HENkaku, for example) with VitaShell. It has been tested on a PS Vita 1000.

1. Download `leftseat.vpk` from the [Releases](../../releases) page.
2. Copy it to your Vita (VitaShell's USB or FTP mode works).
3. Open it in VitaShell and install it.

Your best scores and settings are kept in `ux0:data/skyhawk_scores.txt`, so updating the game doesn't lose them.

## Flying

The start menu is a big **Open World** button with a grid of tiles under it: Airport, Hangar, Weather, Time, Challenges, Options, Controls and Quit. Tap a tile, or move with the D-pad or left stick and press ✕. Pick an aircraft in the Hangar, set the airport, weather and time, then start flying.

### AR view (the Hangar on your desk)

In the Hangar, press **Square** (or tap **AR VIEW**) to switch on the Vita's back camera. Lay a card with a thick black square border on your desk and point the camera at it: the aircraft stands on the card. Use the Vita's own AR cards, or print `docs/ar_marker_a4.png` at 100% (a bigger card gives a steadier aircraft). Keep the whole black border in view and the card flat and matt.

Up / Down change the aircraft, Left / Right turn it on the card, L / R change its size, Triangle steps the camera's field of view (60° is the usual best; if the aircraft seems to slide when you move the Vita, try another), X picks the aircraft for flying, O goes back. The green outline is what the game found; the red edge is the card's top (the aircraft's nose points that way).

A typical takeoff in the K-4: flaps 10, full throttle, a little right rudder to hold the centre line, rotate at about 55 kt and climb at about 74 kt. Come in at about 65 kt with full flaps and flare just above the runway.

| Input | Action |
|---|---|
| Left stick | Pitch and roll |
| Right stick up / down | Throttle |
| Right stick left / right | Rudder |
| L / R | Look left / right (in the chase view, both together swing the camera to the front) |
| D-pad up | Fast forward in free flight: x2, then x5, then normal |
| D-pad down | Wheel brakes on the ground. In the air, trim for level flight at your current speed |
| D-pad left / right | Pitch trim (a tap is a fine step, hold for more) |
| ○ / ✕ | Flaps down / up |
| □ | Landing gear up / down (K-30 and Ashley Twin) |
| △ | Reset the aircraft, or restart the attempt in a challenge |
| Select (tap) | Switch between cockpit and chase camera |
| Select (hold) | Open the chart (the flight pauses) |
| L + R + Select | Invert the pitch axis |
| Select + △ | Centre the gyro controls |
| START | Pause and go back to the menu |
| Select + START | Save a screenshot to `ux0:data/` |

The same list is in the game under **Controls**.

**The chart** shows the airports, the coast, your position and track, and the bearing and distance to each airport. Drag to move it, pinch to zoom, tap an airport to pick it. The D-pad, L / R and ✕ also work.

**Gyro:** in **Options**, set Controls to Gyro and tilt the Vita like a control column. Left and right roll, and the top edge towards you is nose up. It measures from the way you're holding the Vita when the flight starts, and Select + △ centres it again. The tilt adds to the sticks, so you can still use them. If a direction feels backwards, reverse it in Options. The panel on that screen shows what your tilt is asking for.

## Aircraft

| Aircraft | What it is |
|---|---|
| **K-4 Trainer** | Slow, forgiving piston four-seater. Stalls at about 53 kt clean |
| **K-10 Hauler** | Big turboprop. Cruises at about 175 kt |
| **Jet 1 Sprint** | Tiny single-seat jet with a V-tail and a bubble canopy. Cruises at 210 kt |
| **K-30 Courier** | Twin jet with a T-tail, nose canards and retractable gear. Cruises at about 320 kt |
| **Ashley Twin A22** | Six-seat twin turboprop with pusher props, twin booms, a forward-swept wing and an inverted-V tail. Cruises at about 200 kt |
| **Ashley Single A11** | Four-seat single pusher with a swept wing, twin booms and an H-tail. Cruises at about 150 kt |

Each has its own model, cockpit, handling, gauges and best scores. The flight model covers lift, drag, thrust, stalls, flaps, trim, fuel, engine torque and adverse yaw. The wheels are springs and dampers, so touchdowns are gradual and bounces happen. A hard or sideways landing, or hitting a wingtip, tail or propeller, is a crash.

## The world

There are four airports, and one more place you have to find.

| Airport | Where | Notes |
|---|---|---|
| **Knowles Field (KAK)** | Home | Marked runway, lights, PAPI, terminal and hangars |
| **Redrock Municipal (RDR)** | About 16 NM north-north-east | Runway 07/25 in red desert, with a beacon you can see at night |
| **Pinecrest Airstrip (PNC)** | About 11 NM west-north-west | A short grass strip in a forested valley. Hard to spot, so use the chart |
| **Lana Lake Regional (LLK)** | About 8 NM south | Runway 16/34 on the shore of a lake |

In between there's a mountain pass, hills, towns, a highway, a river and some roads with bridges. The last place isn't on the chart or in any list. Follow the coast on the chart and see what's out there. Once you fly close, it's added to the chart and the start menu.

## Challenges

Each one is scored out of 100 on the touchdown: sink rate, distance from the centre line, touchdown point, and heading and bank, with penalties for bounces. A crash scores 0. Your best is saved. The weather and time of day you picked apply to all of them.

| Challenge | What happens |
|---|---|
| **Landing** | Start on final approach, a little off line and height |
| **Engine out** | The engine quits at about 1,500 ft. Glide in and land |
| **Crosswind** | A strong wind blows across the runway. Crab in, then straighten out before touchdown |
| **Cross-country** | Fly from Knowles Field to Redrock and land |
| **Backcountry** | The same trip to the Pinecrest grass strip. Touch down early |
| **Lake rings** | Fly through seven rings over and around Lana Lake |

## Weather, time and sound

Weather can be clear, breezy, overcast, fog, rain, storm or random (which drifts over time). Wind, gusts and turbulence move the aircraft, fog cuts visibility, and storms have lightning. Time of day can be day, dusk or night, with lit airports and a landing light after dark.

All sound is generated while you play: piston, turboprop and jet engines, wind noise, a stall horn, rain and thunder, tyres, brakes and menu clicks. The cockpit is quieter than the chase view. Volume is in Options and is remembered.

## Building from source

You need [VitaSDK](https://vitasdk.org/) with the `vita2d` package and CMake. Python 3 is optional (it cleans the icons before packaging).

```sh
git clone https://github.com/Av8t4/leftseat.git
cd leftseat
mkdir build && cd build
cmake ..
bash ../fixpad.sh      # builds leftseat.vpk; use this instead of plain make
```

The result is `build/leftseat.vpk`. You can also let GitHub build it: every push runs an Action, and the `leftseat-vpk` artifact on the Actions tab is the VPK. `GITHUB_GUIDE.md` walks through releases.

If the build says `Cannot allocate ... bytes for SCE data at end of segment 0`, that's what `fixpad.sh` is for. It grows the padding in `src/pad.c` until it fits, so use it instead of plain `make`. If the app icon shows up white, `tools/strip_png.py` removes the PNG blocks that cause it, and CMake runs it for you.

The game writes `ux0:data/skyhawk.log` with startup lines, a crash reason and frame timings.

## Reporting a problem

Open an issue and tell me what you were doing (aircraft, airport, weather, time), what you expected, what happened, your Vita model and firmware, the game version, and attach `ux0:data/skyhawk.log`.

## Project layout

`src/main.c` includes the other files in order, so the game builds as one unit.

```
src/main.c         entry point and main loop
src/defs.inc       constants, types and global state
src/aircraft.inc   the numbers that make each aircraft handle differently
src/flight.inc, update.inc, gear.inc    the flight model, per-frame update, landing gear
src/camera.inc, clip2d.inc, batch.inc   the software 3D pipeline
src/world.inc, scenery.inc, airports.inc, spaceport.inc   the world
src/models.inc, modelkit.inc, k10.inc, jet1.inc, k30.inc, a22.inc, a11.inc   aircraft models
src/scene.inc, sky.inc, night.inc, weather.inc   drawing a frame, weather and lighting
src/panel.inc, overlays.inc   instrument panel and messages
src/challenge.inc  challenges, scoring and the save file
src/menu.inc, chart.inc, gyro.inc   menus, touch chart, tilt controls
src/soundcore.inc, sound.inc   the sound engine
```

## If the frame rate dips

These are the first things to turn down: `WORLD_DETAIL` and `TILE_R` in `defs.inc`, `TREE_LIMIT` in `scenery.inc`, and `MODEL_DETAIL` in `k10.inc`.

## License

MIT, see [LICENSE](LICENSE).
