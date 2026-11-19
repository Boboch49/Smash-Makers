#pragma once
#include <stddef.h>
#include <stdint.h>
namespace smash_assets {
// Input must be one complete frame read from animations.rgb565.rle.
// The caller supplies drawPixel(x,y,color565); no ESP32 library dependency.
template<class DrawPixel>
bool drawFrame(const uint8_t* data, size_t length, DrawPixel drawPixel) {
    if (!data || length % 4) return false;
    size_t pixels = 0;
    // Validate the full frame before drawing to avoid partially drawn malformed data.
    for (size_t i = 0; i < length; i += 4) {
        uint16_t count = uint16_t(data[i]) | (uint16_t(data[i+1]) << 8);
        if (!count || count > 4096 - pixels) return false;
        pixels += count;
    }
    if (pixels != 4096) return false;
    size_t position = 0;
    for (size_t i = 0; i < length; i += 4) {
        uint16_t count = uint16_t(data[i]) | (uint16_t(data[i+1]) << 8);
        uint16_t color = uint16_t(data[i+2]) | (uint16_t(data[i+3]) << 8);
        while (count--) {
            drawPixel(uint16_t(position % 128), uint16_t(position / 128), color);
            ++position;
        }
    }
    return true;
}
} // namespace smash_assets
