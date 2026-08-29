# Budgets and Limits

| Limit | Value | What happens when you exceed it |
| --- | --- | --- |
| Registered API lights | 64, shared by every mod | `PS_LightCreate` returns `PS_INVALID_LIGHT` and a warning is written to `ProperShaders.log` once. |
| Lights rendered per frame | 127, shared with the game's own lights | API lights are added last, so they take only budget the game did not want. Among themselves they are ranked by camera distance, so the ones that drop out are the far ones. |
| Lights in volumetric fog | 12, shared with the game's own | Ranked by brightness and camera distance together, so a bright API light near the camera can win a slot from a distant game light. |
| Volumetric beams | 127, shared with the game's own | API beams are added after the game's. |

Fog lights are also culled by radius: a light whose radius is under 0.5 units, or over
the user's configured maximum (80 units by default), is skipped by the fog pass. This
is not arbitrary — the fog in-scatter is an analytic integral along the view ray, and
an oversized radius turns one light into a screen-sized wall of glow. Such a light
still lights surfaces normally; it just gets no fog glow.

Practical advice: register the lights you need and keep them, rather than creating and
destroying them per frame. The budget is per *frame*, not per light, and a light that
loses its slot this frame may well get one back the next.
