/*
    Proper Shaders - modding API example
    ------------------------------------

    Press F7 in game to toggle a spot light in the middle of Grove Street.
    The light points straight down and glows in the volumetric fog.

    What this example is really showing:

      1. Loading the API at runtime, and running harmlessly when Proper
         Shaders is not installed at all.
      2. Creating a light and keeping its handle.
      3. Destroying it again through that handle.

    The only file you need from Proper Shaders is ProperShadersAPI.h, sitting
    next to this one. Nothing is linked against Proper Shaders - the API is
    resolved with GetProcAddress - so this .asi loads and runs normally on a
    machine without it.

    See API.md for the full API documentation.

    Built with the Plugin-SDK: https://github.com/DK22Pac/plugin-sdk
*/

#include "plugin.h"
#include "common.h"
#include "CMessages.h"

#include "ProperShadersAPI.h"

using namespace plugin;

class ProperShadersLightExample {
public:
    // ---- What the example does, all in one place -------------------------

    // Middle of Grove Street.
    static constexpr float LIGHT_X = 2489.0f;
    static constexpr float LIGHT_Y = -1666.0f;
    static constexpr float LIGHT_Z = 17.0f;

    static constexpr int TOGGLE_KEY = VK_F7;

    // ---- State -----------------------------------------------------------

    // Every API entry point, filled in by PS_ApiLoad(). Members stay null when
    // Proper Shaders is missing, so `if (ms_api.LightCreate)` is the only
    // guard this example ever needs.
    static PS_Api ms_api;

    // The API is looked up once, lazily. See EnsureApiLoaded().
    static bool ms_apiLoadAttempted;

    // What PS_LightCreate() gave us, and the only thing needed to change or
    // remove the light later. PS_INVALID_LIGHT means "no light right now",
    // which is also what makes it the toggle state.
    static PS_LightHandle ms_light;

    // KeyPressed() reports the key being HELD, so a raw test would toggle the
    // light dozens of times per second. Toggling on the rising edge instead.
    static bool ms_keyWasDown;

    //-------------------------------------------------------------------------
    // Loading the API
    //
    // Deliberately lazy rather than done in the constructor: this class is
    // constructed while the .asi is being loaded, and at that point the loader
    // may not have brought ProperShaders.asi up yet. Doing it on first use is
    // the simplest way to be sure everything is in memory.
    //
    // (Events::initGameEvent works too, if you would rather load it up front.)
    //-------------------------------------------------------------------------
    static bool EnsureApiLoaded() {
        if (!ms_apiLoadAttempted) {
            ms_apiLoadAttempted = true;

            if (!PS_ApiLoad(&ms_api)) {
                // Proper Shaders is not installed. ms_api is zeroed, so every
                // call below is guarded by a null check and nothing happens.
                return false;
            }

            // Version check. Not strictly needed for this example - it only
            // uses version 1 calls, which will always be there - but this is
            // where you would refuse to use a feature the installed build is
            // too old to have.
            if (ms_api.version < 1) {
                return false;
            }
        }

        return ms_api.LightCreate != nullptr;
    }

    //-------------------------------------------------------------------------
    // Turning the light on
    //-------------------------------------------------------------------------
    static void TurnLightOn() {
        PS_LightDesc desc;

        // ALWAYS the first thing you do with a PS_LightDesc. It zeroes the
        // struct, fills in sane defaults, and writes the structSize field the
        // API uses to stay compatible across versions. A description that did
        // not go through it is rejected.
        PS_LightDescInit(&desc);

        desc.type = PS_LIGHT_SPOT;

        desc.position[0] = LIGHT_X;
        desc.position[1] = LIGHT_Y;
        desc.position[2] = LIGHT_Z;

        // Straight down. Points away from the light, and is normalised for
        // you, so any non-zero length would do.
        desc.direction[0] = 0.0f;
        desc.direction[1] = 0.0f;
        desc.direction[2] = -1.0f;

        // How far the light reaches, and how wide the cone opens. spotAngle is
        // the OUTER HALF-angle, in degrees.
        desc.radius = 25.0f;
        desc.spotAngle = 70.0f;

        // Warm white. Colour components are clamped to 0..1 - to go brighter
        // than white, raise intensity instead of pushing them past 1.
        desc.color[0] = 0.4f;
        desc.color[1] = 1.0f;
        desc.color[2] = 0.2f;
        desc.intensity = 0.5f;

        // Let it light up the fog around it: the soft glowing ball a street
        // lamp gets on a foggy night. fogIntensity scales that glow on its
        // own, so you can have a light that hits the ground hard but only
        // hazes the air a little.
        desc.fogMode = PS_FOGMODE_NORMAL;
        desc.fogIntensity = 1.0f;

        // Optional: a visible cone, the way a headlamp looks in the rain.
        // Left off here because it overlaps with the fog glow above and the
        // two together are usually too bright. Uncomment to see it:
        //
        //   desc.beamMode = PS_BEAMMODE_AUTO;
        //   desc.beamLength = 12.0f;

        ms_light = ms_api.LightCreate(&desc);

        if (ms_light == PS_INVALID_LIGHT) {
            // Either the registry is full (64 lights across every mod) or the
            // description was rejected. In real code this is where you would
            // log it - silently doing nothing is how a missing light turns
            // into an afternoon of debugging.
            ShowMessage("~r~Proper Shaders: could not create the light.");
            return;
        }

        ShowMessage("Grove Street light ~g~ON");
    }

    //-------------------------------------------------------------------------
    // Turning the light off
    //
    // Destroying and recreating is shown here because that is the lifecycle
    // most mods need. For something that toggles rapidly - a blinking light, a
    // muzzle flash - keep the light and call
    //
    //     ms_api.LightSetEnabled(ms_light, 0);
    //
    // instead, which is cheaper and keeps the handle valid.
    //-------------------------------------------------------------------------
    static void TurnLightOff() {
        ms_api.LightDestroy(ms_light);

        // The handle is dead now. Calling anything with it would fail cleanly
        // rather than touch another mod's light, but clearing it is what makes
        // it double as our on/off state.
        ms_light = PS_INVALID_LIGHT;

        ShowMessage("Grove Street light ~r~OFF");
    }

    //-------------------------------------------------------------------------
    // Per-frame
    //-------------------------------------------------------------------------
    static void Process() {
        const bool keyIsDown = KeyPressed(TOGGLE_KEY);
        const bool justPressed = keyIsDown && !ms_keyWasDown;
        ms_keyWasDown = keyIsDown;

        if (!justPressed) {
            return;
        }

        if (!EnsureApiLoaded()) {
            ShowMessage("~r~Proper Shaders is not installed.");
            return;
        }

        if (ms_light == PS_INVALID_LIGHT) {
            TurnLightOn();
        }
        else {
            TurnLightOff();
        }
    }

    //-------------------------------------------------------------------------
    // On-screen confirmation. Nothing to do with the API - it is here so you
    // can tell the key press registered without alt-tabbing to a log.
    //-------------------------------------------------------------------------
    static void ShowMessage(const char* text) {
        CMessages::AddMessageJumpQ(text, 2000, 0, false);
    }

    ProperShadersLightExample() {
        Events::processScriptsEvent += Process;
    }
} properShadersLightExample;

PS_Api         ProperShadersLightExample::ms_api = {};
bool           ProperShadersLightExample::ms_apiLoadAttempted = false;
PS_LightHandle ProperShadersLightExample::ms_light = PS_INVALID_LIGHT;
bool           ProperShadersLightExample::ms_keyWasDown = false;
