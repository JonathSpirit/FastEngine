# FastEngine extensions

Extensions are optional modules built on top of FastEngine. They are not part of the core library:
link only the ones you need.

Extensions are enabled with the `FGE_BUILD_EXTENSIONS` CMake option (`ON` by default).

| Extension | CMake target           | Description                                   |
|-----------|------------------------|-----------------------------------------------|
| ObjChart  | `FastEngine::ObjChart` | Point chart with linear axes and animated points |
| Signals   | `FastEngine::Signals`  | Waveform generator and sampled signal processing |

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

## Signals

A header-only set of tools to generate and process sampled signals. Data can be stored as `float`
(`fge::SignalF`, `fge::SignalGeneratorF`) or `double` (`fge::SignalD`, `fge::SignalGeneratorD`).

- `fge::SignalGenerator`: sum of waveforms (sinusoid, rectangle, sawtooth, triangle, continuous),
  generated as a signal or evaluated in real time with `getValue()`
- `fge::Signal`: list of (time, value) samples with strictly increasing times
- Processing: `resample()`, `derivative()`, `integral()`, `softLimit()`, `softLimitKeepAverage()`, `timeStretch()`
- CSV save/load
- Samples are `fge::Vector2<T>` (x = time, y = value) and can be given directly to a `fge::ObjChart`

```cpp
#include "FastEngine/extensions/C_signalGenerator.hpp"

fge::SignalGeneratorD generator;
generator.addWave({fge::SignalGeneratorD::WaveTypes::WAVE_SINUSOID, 1.0, 2.0}); // amplitude 1, 2 Hz
generator.addWave({fge::SignalGeneratorD::WaveTypes::WAVE_RECTANGLE, 0.3, 5.0});

auto const signal = generator.generate(2.0, 0.01); // 2 seconds, one sample every 10 ms
auto const limited = signal.softLimitKeepAverage(-1.0, 1.0, 4.0);
chart->setPoints(limited.getSamples());
```

See `examples/signals_011` for a complete example.
