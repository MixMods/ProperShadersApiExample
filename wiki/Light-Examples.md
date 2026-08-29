# Light Examples

## A lamp attached to a moving object

Create once, then push the position each frame. Do **not** destroy and recreate a
light every frame — you would burn through the registry and lose the handle's
meaning.

```cpp
PS_LightHandle g_lampLight = PS_INVALID_LIGHT;

void OnLampSpawned(CVector pos)
{
    if (!g_ps.LightCreate) return;

    PS_LightDesc d;
    PS_LightDescInit(&d);
    d.position[0] = pos.x; d.position[1] = pos.y; d.position[2] = pos.z;
    d.radius      = 8.0f;
    d.color[0]    = 1.0f; d.color[1] = 0.85f; d.color[2] = 0.6f;
    d.intensity   = 1.5f;
    d.fogMode     = PS_FOGMODE_NORMAL;
    d.fogIntensity = 0.7f;

    g_lampLight = g_ps.LightCreate(&d);
}

void OnFrame(CVector pos)
{
    if (g_lampLight != PS_INVALID_LIGHT) {
        g_ps.LightSetPosition(g_lampLight, pos.x, pos.y, pos.z);
    }
}

void OnLampDestroyed()
{
    if (g_lampLight != PS_INVALID_LIGHT) {
        g_ps.LightDestroy(g_lampLight);
        g_lampLight = PS_INVALID_LIGHT;
    }
}
```

## A searchlight with a visible beam

```cpp
PS_LightDesc d;
PS_LightDescInit(&d);
d.type         = PS_LIGHT_SPOT;
d.position[0]  = pos.x; d.position[1] = pos.y; d.position[2] = pos.z;
d.direction[0] = dir.x; d.direction[1] = dir.y; d.direction[2] = dir.z;
d.radius       = 40.0f;
d.spotAngle    = 25.0f;
d.intensity    = 3.0f;
d.beamMode     = PS_BEAMMODE_AUTO;   // spot light, so the beam is drawn
d.beamLength   = 30.0f;

PS_LightHandle searchlight = g_ps.LightCreate(&d);

// Later, as it sweeps:
g_ps.LightSetDirection(searchlight, newDir.x, newDir.y, newDir.z);
```

## A one-shot muzzle flash

No cleanup needed: it removes itself.

```cpp
PS_LightDesc d;
PS_LightDescInit(&d);
d.position[0] = muzzle.x; d.position[1] = muzzle.y; d.position[2] = muzzle.z;
d.radius      = 6.0f;
d.color[0]    = 1.0f; d.color[1] = 0.8f; d.color[2] = 0.45f;
d.intensity   = 4.0f;
d.lifetimeMs  = 60;
d.fadeOutMs   = 45;

g_ps.LightCreate(&d);   // handle can be ignored
```

## A blinking light

Toggle the flag rather than churning the registry:

```cpp
g_ps.LightSetEnabled(myLight, isOn ? 1 : 0);
```

## Supporting several API versions

`PS_ApiLoad` leaves a member null when the installed build does not export it, so you
can test the pointer instead of the version number:

```cpp
if (g_ps.LightSetRadius) {
    g_ps.LightSetRadius(myLight, 14.0f);
}
```
