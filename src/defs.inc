// Left Seat: constants, types and global state.
// Part of Left Seat's single translation unit: this file is #included by main.c, in order.

#define SCREEN_W   960
#define SCREEN_H   544
#define VIEW_H     400                 // 3D window height; instrument panel below
#define CX         (SCREEN_W / 2.0f)
#define CY         (VIEW_H / 2.0f)
#define FOCAL      g_focal            // pixels; the tower camera zooms by changing this
#define NEAR_Z     g_near             // meters; very close in the cockpit so the wings show from the seat
#define PI_F       3.14159265f
#define G_ACC      9.81f
#define DT         (1.0f / 60.0f)

#define TILE       400.0f              // checkerboard ground tile size (m)
#define TILE_R     10                  // tiles drawn in each direction
#define MAX_FACES  4800
#define MAX_BOXES  400
#define NUM_MOUNTS 7            // the original ring of mountains around the home airport
#define MAX_MOUNTS 96           // every mountain and hill (they all share one shape and collision test)
#define MAX_MESAS  24
#ifndef WORLD_DETAIL
#define CITY_DETAIL 2           // downtown Knowles: 0 = none, 1 = about half the buildings, 2 = the full city
#define WORLD_DETAIL 2          // 0 = the original world and two airports only, 1 = add hills and ridges, 2 = also towns, mesas and the river
#endif

// Aircraft numbers (lift, drag, thrust, weight, gear ...) are in aircraft.inc.
#define ENABLE_TOWER_VIEW 0     // tower camera is switched off; set to 1 to add it to the Select cycle
#define SHOW_FLIGHT_PATH_MARKER 1   // green circle: where the plane is really heading

typedef struct { float x, y, z; } V3;
typedef struct { float x, y; } P2;
typedef struct { V3 pos, r, u, f; } Camera;
typedef struct { V3 p[4]; int n; unsigned int color; V3 c; float rad, top, maxd; } Face;      // rad: distance from c to the farthest corner, top: height of the highest corner
typedef struct { float x, z, w, d, h; } Box;
typedef struct { float x, z, hb, h; } Mount;
typedef struct { float x, z, hb, h, top; } Mesa;      // flat-topped butte: sloping sides up to a flat top of half-width `top`
typedef struct {
    V3 pos, vel;
    V3 f, r, u;                           // body axes: forward, right, up
    float rollrate, pitchrate, yawrate;   // body angular rates (rad/s)
    float ail, ele, rud, trim, throttle;  // smoothed controls, trim, throttle 0..1
    int   flaps;                          // 0..3 = 0/10/20/30 degrees
    float fuel;                           // gallons remaining
    float alpha, beta;                    // angle of attack, sideslip (rad)
    float gamma_prev, gamma_rate;         // flight-path angle memory for the phugoid damper
    float hdg_g, gpitch, gs;              // ground-roll state
    float hdg, pitch, roll, speed, vs;    // derived each frame for camera / panel
    float gear;                           // landing gear position: 0 = up, 1 = down (retractable aircraft only)
    bool  gear_cmd;                       // true = gear selected down
    float n1;                             // spooled engine power 0..1 (a jet takes a few seconds to answer the throttle)
    float rpm;                            // engine / propeller speed
    bool on_ground, crashed;
    bool touched;                         // wheels have been on the ground since the last flight
} Plane;

static Plane  pl;
static Camera cam;
static Face   faces[MAX_FACES];
static int    nfaces;
static Box    boxes[MAX_BOXES];
static int    nboxes;
static Mount  mounts[MAX_MOUNTS];
static Mesa   mesas[MAX_MESAS];
static int    nmesas;
static int    nmounts = NUM_MOUNTS;
static bool   invert_pitch;
static int    frame_count;
static float  g_focal = 520.0f;
static int    ac_lights = 2;         // aircraft lights (D-pad down + Square): 2 = all on, 1 = navigation lights and beacon, 0 = off
static float  g_near  = 2.0f;
static int    view_mode;        // 0 cockpit, 1 chase, 2 control tower
static float  trk_x[300], trk_z[300];   // breadcrumbs of where you have flown, for the chart
static int    ntrk, trk_i, trk_t;
#define CHART_SCROLL_MAX fmaxf(0.0f, ap_nshown() * 78.0f - 314.0f)
static float  chart_scroll;       // how far the airport list on the chart is scrolled (pixels)
static bool   chart_open;        // the chart is open (hold Select): the flight is paused while it is
static void   chart_begin(void);
static bool   engine_failed;     // the engine has stopped (engine-out challenge), whatever the fuel gauge says
static int    view_banner;      // frames left to show the view name
static float  look_ang;         // head / camera turn from straight ahead (radians, right is positive)
static V3     chase_dir = { 0.0f, 0.0f, 1.0f };
static FILE  *logfile;

