#pragma once
#include <stdint.h>

struct RenderDiagnostics {
    uint32_t renderTotalUs = 0;  // time spent in render() on Core 1
    uint32_t flushUs       = 0;  // single-core: time inside flush()
                                 // dual-core: time display task spent in flush()
};

struct EngineDiagnostics {
    uint32_t frameTotalUs  = 0;  // full gameTick() wall time
    uint32_t updatePhaseUs = 0;  // everything in gameTick() before render()
    uint32_t renderPhaseUs = 0;  // time spent in m_renderer.render()
    uint16_t fpsAvg1s      = 0;  // average FPS over last 1 second
};
