# Troubleshooting

**`PS_ApiLoad` returns 0.**
Proper Shaders is not loaded *yet*, or not installed. Check that you are calling it
after game startup rather than from `DllMain`.

**`PS_LightCreate` returns `PS_INVALID_LIGHT`.**
Either the registry is full (check `PS_LightGetCount()` against
`PS_LightGetCapacity()`) or the description was rejected. The usual causes: you did
not call `PS_LightDescInit`, `radius` is zero or negative, or a NaN reached one of the
float fields — a position computed from an uninitialised matrix is the classic one.

**The light does not appear.**
Work down this list:
1. Is the deferred renderer enabled in the user's config? Nothing renders without it.
2. Is `PS_LIGHTFLAG_DISABLED` set, or `intensity` zero?
3. Is the light actually where you think? Try a large `intensity` and a radius of 20
   at the player's position to confirm the plumbing before debugging the placement.
4. Check `ProperShaders.log` for `[API]` lines.

**The light appears but is much dimmer than I asked for.**
The map's per-area light tweaks are scaling it. Set
`PS_LIGHTFLAG_IGNORE_AREA_TWEAKS` if your light must keep exactly the intensity you
gave it.

**The fog glow is missing but the light works.**
Either the user has volumetric fog off, or the light lost the per-frame fog budget, or
its radius is outside the fog pass's accepted range (under 0.5 units, or over the
configured maximum). Reducing a very large radius and raising `intensity` usually
fixes it.

**My light survived a config reload.**
That is deliberate. Reloading Proper Shaders' settings in-game does not clear the
registry, because your mod still holds the handles and has no way to know a reload
happened.
