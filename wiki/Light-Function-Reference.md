# Light Function Reference

Every function is `extern "C" __cdecl` and exported under exactly the name below.
All of them are safe to call at any time from the main thread, and the light registry
is internally locked, so calls from another thread are safe too.

## `PS_LightHandle PS_LightCreate(const PS_LightDesc* desc)`

Registers a light. Returns its handle, or `PS_INVALID_LIGHT` (`0`) if the description
was rejected or the registry is full.

## `int PS_LightDestroy(PS_LightHandle light)`

Unregisters the light. Returns `1` if it existed. Destroying an already-destroyed or
never-valid handle is harmless and returns `0`.

## `int PS_LightExists(PS_LightHandle light)`

`1` while the handle refers to a live light. Note that a light with a `lifetimeMs`
can expire on its own, so this can turn `0` without you destroying anything.

## `int PS_LightUpdate(PS_LightHandle light, const PS_LightDesc* desc)`

Replaces **every** property of an existing light. Returns `1` on success, `0` for a
stale handle or an invalid description.

> This includes the lifetime: `PS_LightUpdate` **restarts the countdown**. To just
> move or recolour a light, use the setters below, which leave the clock alone.

## `int PS_LightGet(PS_LightHandle light, PS_LightDesc* desc)`

Fills `desc` with the light's current properties. Call `PS_LightDescInit(&desc)`
first — its `structSize` is what tells Proper Shaders how much of your struct it may
write, which is what keeps the call safe against a newer build with a bigger struct.

Values come back **normalised**: colours clamped, direction unit-length, angles
clamped to their valid range. So a get/set round trip will not necessarily hand back
the exact numbers you passed in.

## Per-property setters

Cheap, and intended to be called every frame. Each returns `1` on success and `0` for
a stale handle.

| Function | Notes |
| --- | --- |
| `int PS_LightSetPosition(PS_LightHandle, float x, float y, float z)` | Rejects non-finite values. |
| `int PS_LightSetDirection(PS_LightHandle, float x, float y, float z)` | Normalised for you. Rejects a zero-length direction. |
| `int PS_LightSetColor(PS_LightHandle, float r, float g, float b, float intensity)` | Colour clamped to 0–1, intensity to ≥ 0. |
| `int PS_LightSetRadius(PS_LightHandle, float radius)` | Must be positive. |
| `int PS_LightSetEnabled(PS_LightHandle, int enabled)` | Clears or sets `PS_LIGHTFLAG_DISABLED`. |

## Diagnostics

| Function | Returns |
| --- | --- |
| `int PS_LightGetCount(void)` | How many API lights are registered right now, across every mod. |
| `int PS_LightGetCapacity(void)` | How many the registry can hold in total. |
