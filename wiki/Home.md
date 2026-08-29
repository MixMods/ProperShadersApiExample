# Proper Shaders — Modding API

**Note: Keep in mind that this documentation is written almost entirely by AI (because I hate writing wikis/documentation).**

Proper Shaders exposes a C API so other mods (`.asi` / `.dll`) can work with its
renderer. It currently covers two things:

- **[Lights](Lights.md)** — point and spot lights you create, move, recolour and
  remove at runtime, with per-light control over how they interact with volumetric
  fog and volumetric light beams.
- **[State queries](State-Queries.md)** — what Proper Shaders is doing right now:
  which features the user has on, which pass is rendering, and the render targets
  themselves.

The API is resolved at runtime with `GetProcAddress`. Nothing is linked, so a mod
using it still loads and runs normally on a machine where Proper Shaders is not
installed — the effects simply do not appear.

**Current API version: 2**

---

## Start here

Copy **`ProperShadersAPI.h`** into your project. It is a single self-contained
header with no dependencies beyond the Windows headers, and it works from both C
and C++. That is the whole integration — there is nothing to link.

```cpp
#include "ProperShadersAPI.h"

PS_Api ps;
if (PS_ApiLoad(&ps)) {
    PS_LightDesc d;
    PS_LightDescInit(&d);
    d.position[0] = x; d.position[1] = y; d.position[2] = z;
    d.radius = 12.0f;
    d.intensity = 2.0f;
    d.fogMode = PS_FOGMODE_NORMAL;

    PS_LightHandle light = ps.LightCreate(&d);
}
```

→ [Getting Started](Getting-Started.md) for the details, or
[Example Project](Example-Project.md) for a complete mod you can build and run.

---

## Pages

| | |
| --- | --- |
| [Getting Started](Getting-Started.md) | Loading the API, and behaving well when Proper Shaders is absent. |
| [Example Project](Example-Project.md) | A complete Plugin-SDK mod using the API. |
| [Lights](Lights.md) | Creating lights, and every field of `PS_LightDesc`. |
| [Light Function Reference](Light-Function-Reference.md) | Every light entry point. |
| [Light Examples](Light-Examples.md) | Worked examples for common cases. |
| [Budgets and Limits](Budgets-and-Limits.md) | How many lights you get, and what happens past that. |
| [State Queries](State-Queries.md) | Enabled features, current render pass, render targets. |
| [Requirements and Fallbacks](Requirements-and-Fallbacks.md) | What the user must have enabled for each effect. |
| [Versioning and Compatibility](Versioning-and-Compatibility.md) | The two version numbers, and the ABI promise. |
| [Troubleshooting](Troubleshooting.md) | When something does not show up. |
