# Requirements and Fallbacks

API lights are rendered by Proper Shaders' own passes, so each part of a light needs
the feature that draws it to be enabled by the **user**, in their configuration:

| What you asked for | Needs |
| --- | --- |
| Surface lighting | **Deferred Renderer** and **Deferred Lights** enabled. |
| `fogMode` | **Volumetric Fog** enabled, with its local lights option on. |
| `beamMode` | **Volumetric Lights** enabled. |

None of this changes the API's behaviour: `PS_LightCreate` still succeeds and the
handle stays valid whatever the user has turned on, so your mod needs no branches for
it. The light simply is not drawn.

Two things worth knowing so a missing light does not send you hunting:

- With the deferred renderer off, an API light stops illuminating surfaces entirely —
  a fog glow is then the only part of it that can still show. Proper Shaders writes
  one warning to `ProperShaders.log` the first time a mod creates a light on such a
  configuration.
- Volumetric beams are switched off inside interiors, for the game's own lights and
  for API lights alike.
