#include "tvgGlFill.h"

GlLinearBlock::GlLinearBlock(tvg::LinearGradient* gradient)
{
    gradient->linear(&startPos[0], &startPos[1], &endPos[0], &endPos[1]);

    const tvg::Fill::ColorStop* colorStops = nullptr;

    nStops = static_cast<uint32_t>(gradient->colorStops(&colorStops));

    if (nStops == 0) {
        return;
    }

    // FIXME to handle index out of bounds error
    if (nStops > 4) {
        nStops = 4;
    }

    for (int32_t i = 0; i < nStops; i++) {
        stopPoints[i] = colorStops[i].offset;

        stopColors[i * 4 + 0] = colorStops[i].r / 255.f;
        stopColors[i * 4 + 1] = colorStops[i].g / 255.f;
        stopColors[i * 4 + 2] = colorStops[i].b / 255.f;
        stopColors[i * 4 + 3] = colorStops[i].a / 255.f;
    }
}

GlRadialBlock::GlRadialBlock(tvg::RadialGradient* gradient)
{
    gradient->radial(&centerPos[0], &centerPos[1], &radius[0]);
    radius[1] = 0;

    const tvg::Fill::ColorStop* colorStops = nullptr;

    nStops = static_cast<uint32_t>(gradient->colorStops(&colorStops));

    if (nStops == 0) {
        return;
    }

    // FIXME to handle index out of bounds error
    if (nStops > 4) {
        nStops = 4;
    }

    for (int32_t i = 0; i < nStops; i++) {
        stopPoints[i] = colorStops[i].offset;

        stopColors[i * 4 + 0] = colorStops[i].r / 255.f;
        stopColors[i * 4 + 1] = colorStops[i].g / 255.f;
        stopColors[i * 4 + 2] = colorStops[i].b / 255.f;
        stopColors[i * 4 + 3] = colorStops[i].a / 255.f;
    }
}