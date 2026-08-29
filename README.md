# Proper Shaders API — example project

A minimal, complete Plugin-SDK mod that uses the Proper Shaders modding API.

**Press F7 in game** to toggle a spot light in the middle of Grove Street. It points
straight down and glows in the volumetric fog. Press it again to remove the light.

**Full API documentation: [the wiki](https://github.com/MixMods/ProperShadersApiExample/wiki).**

## What it demonstrates

- Loading the API at runtime, and **running harmlessly when Proper Shaders is not
  installed** — the mod still loads, the light just never appears.
- Creating a light with `PS_LightCreate` and keeping its handle.
- Removing it again with `PS_LightDestroy`.
- The per-light options that matter most: spot cone, colour and intensity, and
  volumetric fog participation.

`Main.cpp` is heavily commented and is meant to be read top to bottom — it is the
example, not just a test.

The API also has a second half this example does not touch: **state queries** — which
features the user has enabled, which render pass is currently running, the shadow
influence for the frame, and the deferred render targets themselves. See
[State Queries](https://github.com/MixMods/ProperShadersApiExample/wiki/State-Queries)
in the wiki.

## Files

| File | |
| --- | --- |
| `Main.cpp` | The example itself. |
| `ProperShadersAPI.h` | The API header, copied verbatim from Proper Shaders. **This is the only file you need to take into your own project.** |
| `ProperShadersApiExample.sln` / `.vcxproj` | Visual Studio project. |
| `wiki/` | Source of the [documentation wiki](https://github.com/MixMods/ProperShadersApiExample/wiki). Edit here, not in the wiki UI — see [below](#editing-the-documentation). |

## Building

You need the [Plugin-SDK](https://github.com/DK22Pac/plugin-sdk) and the
`PLUGIN_SDK_DIR` environment variable pointing at it — the same setup any Plugin-SDK
mod uses. If you also set `GTA_SA_DIR`, the post-build step copies the built `.asi`
straight into your game's `scripts` folder.

Open `ProperShadersApiExample.sln` and build the **Release / GTA-SA** configuration,
or from a command line:

```bash
msbuild "ProperShadersApiExample.sln" -p:Configuration=Release -p:Platform=GTA-SA
```

The output is `bin\GTA-SA\Release\ProperShadersApiExample.SA.asi`. Drop it in your
game's `scripts` folder (or anywhere an ASI loader picks it up) alongside a working
Proper Shaders install.

## Trying it out

1. Load a save and goto Grove Street; is where you will see it.
2. Press **F7**. A message confirms the light was created.
3. For the fog glow, the user config needs **Volumetric Fog** enabled; for the light
   to hit the ground at all, it needs the **Deferred Renderer**. See
   [Requirements and Fallbacks](https://github.com/MixMods/ProperShadersApiExample/wiki/Requirements-and-Fallbacks)
   if nothing shows up.

## Using it in your own mod

Copy `ProperShadersAPI.h` into your project. That is the whole integration — there is
nothing to link, and no build-time dependency on Proper Shaders:

```cpp
#include "ProperShadersAPI.h"

PS_Api ps;
if (PS_ApiLoad(&ps)) {
    PS_LightDesc d;
    PS_LightDescInit(&d);
    d.position[0] = x; d.position[1] = y; d.position[2] = z;
    d.radius = 12.0f;
    d.intensity = 2.0f;

    PS_LightHandle light = ps.LightCreate(&d);
}
```

Call `PS_ApiLoad` **after** the game has loaded its mods, not from `DllMain` — see the
comments on `EnsureApiLoaded()` in `Main.cpp`.

## Editing the documentation

The wiki is generated from the **`wiki/` folder in this repository**, so the docs are
reviewed and versioned like the rest of the code. Do not edit pages in the GitHub wiki
UI — the next sync overwrites them.

Edit the `.md` files under `wiki/`, push to `main`, and the
[Sync Wiki](.github/workflows/sync-wiki.yml) workflow publishes them. One file per
page: `Home.md` is the landing page, `_Sidebar.md` is the navigation shown beside every
page, and a file named `Some-Page.md` becomes the page *Some Page*.

Links between pages are written as ordinary relative links (`[Lights](Lights.md)`) so
they work both when browsing `wiki/` here and in the published wiki, where the action
rewrites them.

---

Built with the [Plugin-SDK](https://github.com/DK22Pac/plugin-sdk).
