# Versioning and Compatibility

- **Exported functions are frozen.** Once released, a function's name, signature and
  calling convention never change. The API grows by *adding* functions.
- **`PS_LightDesc` grows by appending.** New fields are only ever added at the end,
  and the `structSize` field tells Proper Shaders which version of the struct you
  compiled against. A mod built against version 1 of this header keeps working, with
  no recompile, on later versions — the fields it never knew about take their default
  values.
- **This means you must never set `structSize` by hand**, and never reorder or resize
  the fields in your copy of the header. `PS_LightDescInit` is the only thing that
  should write it.
- **`PS_GetApiVersion` never changes.** It is the one export you can rely on to
  discover everything else.

## The two version numbers

They answer different questions, and you almost always want the first:

| | |
| --- | --- |
| `PS_GetApiVersion()` | What the API *supports*. Moves only when the API gains something. Check this before using a call added in a later version. |
| `PS_GetVersion()` | Which build of Proper Shaders is installed. Moves every release. Check this to work around a bug in a specific build, or to require a minimum. |

To use anything added after the version you built against, either compare
`PS_Api::version` or just test the function pointer — `PS_ApiLoad` leaves it null when
the installed build does not export it:

```cpp
if (ps.GetGlobalShadowsInfluence) {
    float influence = ps.GetGlobalShadowsInfluence();
}
```

Testing the pointer is the more robust of the two, because it keeps working if a call
is ever backported.

## Version history

| API version | |
| --- | --- |
| 1 | Lights. |
| 2 | State queries: version and lifecycle, enabled features, current render pass, shadow influence, render targets. |
| 3 | `PS_LIGHTFLAG_CAST_SHADOW`: a light can ask for a shadow. |
