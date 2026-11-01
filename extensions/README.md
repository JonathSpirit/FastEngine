# FastEngine extensions

Extensions are optional modules built on top of FastEngine. They are not part of the core library:
link only the ones you need.

Extensions are enabled with the `FGE_BUILD_EXTENSIONS` CMake option (`ON` by default).

| Extension | CMake target           | Description                                   |
|-----------|------------------------|-----------------------------------------------|
| ObjChart  | `FastEngine::ObjChart` | Point chart with linear axes and animated points |

## ObjChart

A header-only point chart (scatter plot) object. Data can be stored as `float` (`fge::ObjChartF`)
or `double` (`fge::ObjChartD`).

- Linear X/Y axes, automatic ticks or fixed ranges
- Optional grid and tick labels (when a font is set)
- Optional straight lines linking consecutive points (in index order), shown by default
- Optional animation: a modified point gently moves toward its new position
- Fixed size or `fge::DynamicSize` (e.g. fill a `fge::ObjWindow`)

```cpp
#include "FastEngine/extensions/C_objChart.hpp"

auto* chart = scene.newObject<fge::ObjChartD>();
chart->setFont("base");
chart->setSize({400.0f, 250.0f});
chart->addPoint({1.0, 2.5});
chart->setPoint(0, {1.0, 4.0}); // animated
```

See `examples/objChart_010` for a complete example.
