# State Queries

Read-only calls that answer *what is Proper Shaders doing right now*. Each one is a
single read of a value the renderer already maintains, so they are cheap enough to
poll every frame.

They are deliberately flat — one function per fact, plain `int` / `float` / pointer
returns, no structs and no enums — so you resolve only the handful you need and a
missing one costs you nothing but a null pointer. **Booleans are returned as `int`**:
`1` for true, `0` for false.

> All of these describe a frame in progress, so they only mean anything when read on
> the game thread. The render-pass ones in particular change many times per frame and
> should be read from inside a render hook.

## Version and lifecycle

| Function | |
| --- | --- |
| `int PS_GetApiVersion(void)` | Version of the **API**. The only export you may assume exists — its name and signature will never change. `PS_ApiLoad` calls it for you and stores the result in `PS_Api::version`. |
| `int PS_GetVersion(void)` | Version of **Proper Shaders itself**, as `major*10000 + minor*100 + patch` — so 1.2.3 reads as `10203`. Compare with `<`. |
| `int PS_IsInitialized(void)` | `1` once first-frame initialisation has finished. Before that, the render state and buffers below are not meaningful yet. |
| `void PS_ReloadSettings(void)` | Re-reads the ini files, the same thing the `PSRLD` cheat does. |

> ⚠️ **`PS_ReloadSettings` is experimental.** Reloading is not fully supported yet: it
> re-runs feature configuration mid-frame, and not every feature rebuilds its device
> resources cleanly from there. Useful while developing; do not call it from a shipped
> mod.

## Enabled features

These reflect the user's configuration, so they are fixed for the run (until a settings
reload). Branch on them when deciding whether an effect of your own is worth doing.

| Function | |
| --- | --- |
| `int PS_IsUsingDeferredRenderer(void)` | |
| `int PS_IsUsingDeferredLights(void)` | |
| `int PS_IsUsingGlobalReflections(void)` | |
| `int PS_IsUsingHDR(void)` | |
| `int PS_IsUsingReversedDepthBuffer(void)` | Depth is reversed, so near = 1. Matters if you sample the depth buffer yourself. |
| `int PS_IsUsingGlobalShadows(void)` | Directional (sun/moon) shadows specifically. |
| `int PS_IsUsingAnyShadows(void)` | True if *any* shadow system is on: global shadows, skylight, or headlights. |

Which pipeline replaced the game's own, per object class. **`0` always means the stock
unlit PC pipeline** — i.e. Proper Shaders is not shading that class, and features like
shadows will not apply to it.

| Function | Values |
| --- | --- |
| `int PS_GetWorldPipeline(void)` | `0` = PC unlit, `1` = PS2 PBR |
| `int PS_GetVehiclePipeline(void)` | `0` = PC unlit, `1` = PS2 lit, `2` = Modern PBR |
| `int PS_GetSkinPipeline(void)` | `0` = PC unlit, `1` = PC lit |

## Current render pass

| Function | |
| --- | --- |
| `int PS_IsDeferredPass(void)` | |
| `int PS_IsForwardPass(void)` | |
| `int PS_IsMostCommonRenderCall(void)` | See below. |
| `int PS_IsRenderingScene(void)` | |
| `int PS_IsRenderingPostFX(void)` | |
| `int PS_IsCapturingGlobalShadows(void)` | Building the sun/moon shadow map. |
| `int PS_IsCapturingLocalShadows(void)` | Building the headlight shadow map. |
| `int PS_IsCapturingSkylight(void)` | |
| `int PS_IsCapturingReflections(void)` | Building the reflection capture. |
| `int PS_IsCapturingDepth(void)` | |
| `int PS_IsDepthCapturedThisFrame(void)` | |

**`PS_IsMostCommonRenderCall` is the one worth knowing about.** Proper Shaders renders
the world several times per frame — shadow maps, reflection captures, and so on — and a
render hook of yours fires for every one of them. This is `1` for exactly one call per
frame: the real scene pass the player sees. Gate on it to run per-frame work once
instead of five times:

```cpp
void MyRenderHook() {
    if (ps.IsMostCommonRenderCall && !ps.IsMostCommonRenderCall()) {
        return;   // a shadow or reflection re-render, skip it
    }
    ...
}
```

## Lighting

### `float PS_GetGlobalShadowsInfluence(void)`

How much the directional shadows are actually shaping the scene this frame, `0..1`.

Returns `0` whenever they cannot contribute at all — switched off, suppressed for the
frame, or indoors — and otherwise the night fade scaled by the current sun strength.

This is a different question from `PS_IsUsingGlobalShadows()`: that one says the user
has the feature enabled, this one says whether it is doing anything *right now*. Use it
to fade an effect of your own in and out along with the game's shadows.

## Render targets

Every one of these returns a `IDirect3DTexture9*` (typed as `PS_Texture`, which is
`void*` when the header is compiled as C).

| Function | |
| --- | --- |
| `PS_GetDepthBuffer()` | INTZ depth. Note `PS_IsUsingReversedDepthBuffer()`. |
| `PS_GetNormalsLayerBuffer()` | World-space normals. Written by the forward pipelines too. |
| `PS_GetMotionVectorBuffer()` | |
| `PS_GetShadowsMask()` | |
| `PS_GetDeferredAlbedoBuffer()` | |
| `PS_GetDeferredMaterialsBuffer()` | Roughness / metallicness / AO. |
| `PS_GetDeferredEmissiveBuffer()` | |
| `PS_GetDeferredLightingBuffer()` | HDR light accumulation. |
| `PS_GetDeferredLightsBuffer()` | |

The five `Deferred*` buffers are null unless the deferred renderer is on.

**These are borrowed pointers, valid only for the call that asked for it:**

- **Re-fetch every time.** They are recreated on a device reset and on any resolution
  change — a pointer cached last frame may be dangling.
- **Always null check.** A buffer is null when the feature that owns it is off, or has
  not run yet this frame.
- **Never `Release()` and never `AddRef()`.** You do not own them; releasing one will
  take the renderer down with it.

Reading one outside a render hook is meaningless, and writing to one is not supported.
