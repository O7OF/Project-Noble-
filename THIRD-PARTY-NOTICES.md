# Third-party notices

This repository is proprietary (see [LICENSE](LICENSE)). It contains code and
libraries that carry their own terms, reproduced or referenced below.

## Previously open-source code (MIT)

The Project Noble / Halo: Reach Community Edition source up to commit
`1d01c3c` (the 0.9.16.35 base) was published under the MIT License. That
grant remains valid for the versions distributed under it. The notice:

```
MIT License

Copyright (c) 2026 Project Noble contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Bundled / fetched libraries

These are pulled in by CMake `FetchContent` at configure time and are not
modified. Each keeps its own license; the upstream license text ships with the
downloaded source.

| Library | License |
|---|---|
| Dear ImGui | MIT |
| spdlog | MIT |
| nlohmann/json | MIT |
| MinHook | BSD 2-Clause |
| Opus | BSD 3-Clause |

## Game data and trademarks

Halo: Reach, Halo: The Master Chief Collection, and all associated game
assets, maps, models, textures, audio, and trademarks are the property of
Microsoft Corporation and 343 Industries. They are not licensed here and are
not distributed with this repository. The client loads `haloreach.dll` and
game data from a locally installed, legitimately owned MCC copy. You must own
the game to use this software.

The menu map, string tables, armour exports and other extracted game data that
the build can embed are generated locally from the user's own MCC install and
are excluded from this repository by `.gitignore`.
