# Getting Started

Copy **`ProperShadersAPI.h`** into your project. It is a single self-contained header
with no dependencies beyond the Windows headers, and it works from both C and C++.

```cpp
#include "ProperShadersAPI.h"

PS_Api g_ps;

// Call this once, after the game has loaded its mods.
void MyModInit()
{
    if (!PS_ApiLoad(&g_ps)) {
        // Proper Shaders is not installed. g_ps is zeroed, so every
        // g_ps.Something pointer is null and easy to guard against.
        return;
    }
}
```

`PS_ApiLoad` returns `1` when Proper Shaders is present. It fills `g_ps` with one
function pointer per entry point, plus `g_ps.version` — the API version of the
*installed* build, which may be older than the header you compiled against.

> **Call it after Proper Shaders has loaded.** `PS_ApiLoad` looks for an
> already-loaded `ProperShaders.asi`, so calling it from `DllMain` is too early:
> the loader is still bringing the other `.asi` files up. From a plugin-sdk mod use
> an initialisation event; from a raw `.asi`, any point after game startup works.

If you would rather not use the bundled loader, every entry point is exported under
exactly the name this wiki gives it, undecorated, `__cdecl` — so a plain
`GetProcAddress(module, "PS_LightCreate")` works just as well.
