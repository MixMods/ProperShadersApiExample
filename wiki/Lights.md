# Lights

Point and spot lights you create, move, recolour and remove at runtime. Every light
is described by a [`PS_LightDesc`](#ps_lightdesc-reference) and identified afterwards
by the handle you get back.

## Creating a light

```cpp
PS_LightDesc desc;
PS_LightDescInit(&desc);            // ALWAYS call this first

desc.type        = PS_LIGHT_POINT;
desc.position[0] = x;
desc.position[1] = y;
desc.position[2] = z;
desc.radius      = 12.0f;
desc.color[0]    = 1.0f;            // warm orange
desc.color[1]    = 0.6f;
desc.color[2]    = 0.2f;
desc.intensity   = 2.0f;
desc.fogMode     = PS_FOGMODE_NORMAL;   // glow in volumetric fog

PS_LightHandle myLight = g_ps.LightCreate(&desc);
if (myLight == PS_INVALID_LIGHT) {
    // Registry full, or the description was rejected.
}
```

`PS_LightDescInit` zeroes the struct, writes the `structSize` field the API uses for
versioning, and fills in sane defaults (a white omnidirectional lamp, radius 10,
surface lighting only). **Never skip it and never set `structSize` yourself** — a
description that does not carry a valid `structSize` is rejected.

Keep the handle. Everything else — moving the light, changing its colour, removing
it — goes through it:

```cpp
g_ps.LightSetPosition(myLight, x, y, z);    // cheap, fine to call every frame
...
g_ps.LightDestroy(myLight);                 // when you are done with it
```

Handles are **not reused**. Once you destroy a light, its handle stays invalid:
calling any function with it returns `0` rather than quietly driving whatever light
took over the slot. That makes a stale handle a harmless bug instead of a mysterious
one, so you do not need to null it out defensively. (The counter that guarantees this
is 16 bits wide, so a handle could in principle come round again after ~65,000
further light creations land on the same slot. You will not hit it in practice, but
it is the reason to treat a destroyed handle as dead rather than as a permanent
unique id.)

---

## `PS_LightDesc` reference

```c
typedef struct PS_LightDesc
{
    unsigned int structSize;        // set by PS_LightDescInit, do not touch

    int          type;              // PS_LightType
    unsigned int flags;             // PS_LightFlags

    float        position[3];
    float        direction[3];

    float        radius;
    float        color[3];
    float        intensity;

    float        spotAngle;

    int          fogMode;           // PS_FogMode
    float        fogIntensity;

    int          beamMode;          // PS_BeamMode
    float        beamLength;
    float        beamIntensity;

    unsigned int lifetimeMs;
    unsigned int fadeOutMs;
} PS_LightDesc;
```

Every field is validated on the way in. Out-of-range numbers are clamped rather than
rejected; only a description that cannot mean anything at all (a null pointer, a
missing `structSize`, a non-positive radius, a NaN or infinity anywhere) is refused,
and `PS_LightCreate` then returns `PS_INVALID_LIGHT`.

### Type and flags

| Field | Values | Meaning |
| --- | --- | --- |
| `type` | `PS_LIGHT_POINT` (default) | Omnidirectional. `direction` is ignored. |
| | `PS_LIGHT_SPOT` | A cone along `direction`, `spotAngle` wide. |

`flags` is a bit field:

| Flag | Effect |
| --- | --- |
| `PS_LIGHTFLAG_DISABLED` | The light stays registered but stops rendering. Cheaper than destroy/create for something that blinks or is toggled. See also `PS_LightSetEnabled`. |
| `PS_LIGHTFLAG_NO_SURFACE_LIGHTING` | Skip the deferred pass: the light no longer illuminates geometry, but still feeds fog and beams. For something meant to read as a glow in the air rather than as a lamp lighting the street. |
| `PS_LIGHTFLAG_IGNORE_AREA_TWEAKS` | Ignore the per-area light tweaks the map config applies. Proper Shaders dims or brightens lights in specific parts of the map to keep the vanilla map looking right; set this flag for lights that must keep exactly the intensity you asked for, such as a flashlight the player carries or a scripted cutscene light. |

### Placement and shape

| Field | Default | Notes |
| --- | --- | --- |
| `position[3]` | `0,0,0` | GTA world space, in game units (roughly metres). |
| `direction[3]` | `0,0,-1` | Points **away** from the light. Normalised for you, so any non-zero length is fine. Spot lights only. A zero-length direction falls back to straight down. |
| `radius` | `10.0` | Distance at which the light has fallen off to nothing. |
| `spotAngle` | `45.0` | Outer **half**-angle of the cone, in degrees. Clamped to 1–89. Spot lights only. |

`radius` is the light's whole extent, not a "useful" range. Its cost grows with the
screen area it covers, so a small radius with a higher `intensity` is much cheaper
than a huge dim sphere — and it also keeps the light from being culled out of the
[volumetric fog budget](Budgets-and-Limits.md), which rejects oversized radii outright.

### Colour and brightness

| Field | Default | Notes |
| --- | --- | --- |
| `color[3]` | `1,1,1` | Linear RGB, each **clamped to 0–1**. |
| `intensity` | `1.0` | Multiplier on top of the user's global light intensity setting. |

To make a light brighter than white, raise `intensity` — do not push colour
components past 1, they are clamped. `intensity = 1.0` is roughly "as bright as a
normal game light of this radius".

### Volumetric fog

`fogMode` controls whether the light lights up the **fog medium** around it: the soft
glowing ball a street lamp gets on a foggy night. This is the omnidirectional
in-scattering, and it is separate from the visible cone below.

| Value | Meaning |
| --- | --- |
| `PS_FOGMODE_NONE` (default) | Lights surfaces only. Cheapest. |
| `PS_FOGMODE_NORMAL` | Scatters in the fog. |

`fogIntensity` (default `1.0`) scales the fog glow **on its own**, on top of
`intensity`. Use it to tune the glow without changing how the light hits surfaces: a
lamp that should light the pavement strongly but only haze the air a little wants a
full `intensity` and a `fogIntensity` well under 1.

Fog lights are subject to a small per-frame budget and to distance and radius culling;
see [Budgets and limits](Budgets-and-Limits.md).

### Volumetric beam

`beamMode` controls the visible volumetric **cone** — the shaft of a headlamp in the
rain. It is drawn by the Volumetric Lights feature and is independent from `fogMode`.

| Value | Meaning |
| --- | --- |
| `PS_BEAMMODE_NONE` (default) | Never draw a beam. |
| `PS_BEAMMODE_AUTO` | Beam for spot lights; nothing for point lights. |
| `PS_BEAMMODE_ALWAYS` | Beam even for a point light, along `direction`. |

| Field | Default | Notes |
| --- | --- | --- |
| `beamLength` | `0.0` | Length of the cone in game units. `0` uses the length the user configured. |
| `beamIntensity` | `1.0` | Scales the beam **on its own**, on top of `intensity`. |

The cone's *angle* comes from the user's Volumetric Lights configuration, not from
`spotAngle` — that keeps every beam in the scene visually consistent, which is the
whole point of the feature. `spotAngle` still shapes the light that lands on
surfaces.

> Setting both `fogMode` and `beamMode` on the same spot light is allowed, but the
> two effects overlap and the result is usually too bright. For a spot light, prefer
> the beam; for a point light, the fog glow.

### Lifetime

| Field | Default | Notes |
| --- | --- | --- |
| `lifetimeMs` | `0` | Self-destruct timer in gameplay milliseconds. `0` = the light lives until you destroy it. |
| `fadeOutMs` | `0` | Tail of `lifetimeMs` spent fading to black. Clamped to `lifetimeMs`. Ignored when `lifetimeMs` is `0`. |

The clock is gameplay time, so it does not run while the game is paused. Use it for
one-shot lights — a muzzle flash, a spark, an explosion — so a missed cleanup path
cannot leak a light that stays lit forever. A light that expires is removed
automatically, and its handle becomes invalid like any destroyed one.

`fadeOutMs` exists so a timed light goes out smoothly instead of popping. A 400 ms
light with a 300 ms fade holds full brightness for 100 ms and then ramps down.
