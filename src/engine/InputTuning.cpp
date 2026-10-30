#include "engine/InputTuning.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <utility>

namespace noble {
namespace InputTuning {

namespace {
constexpr float kMax = 32767.0f;
}

void ApplyDeadZone(int16_t& x, int16_t& y, float zone) {
    if (zone <= 0.0f) return;
    zone = std::min(zone, 0.95f);
    const float fx = x / kMax, fy = y / kMax;
    const float length = std::sqrt(fx * fx + fy * fy);
    if (length <= zone) {
        x = y = 0;
        return;
    }
    const float scale = std::min(1.0f, (length - zone) / (1.0f - zone)) / length;
    x = static_cast<int16_t>(std::clamp(fx * scale, -1.0f, 1.0f) * kMax);
    y = static_cast<int16_t>(std::clamp(fy * scale, -1.0f, 1.0f) * kMax);
}

void ApplyStickLayout(int layout, int16_t& leftX, int16_t& leftY, int16_t& rightX, int16_t& rightY) {
    if (layout != 1) return;   // 0 = Default; anything else is not a layout we offer yet
    std::swap(leftX, rightX);
    std::swap(leftY, rightY);
}

void ApplyMouseCurve(float& dx, float& dy, float acceleration, float smoothing, float& smoothX, float& smoothY) {
    if (smoothing > 0.001f) {
        smoothing = std::min(smoothing, 0.9f);
        smoothX = smoothX * smoothing + dx * (1.0f - smoothing);
        smoothY = smoothY * smoothing + dy * (1.0f - smoothing);
        dx = smoothX;
        dy = smoothY;
    }
    if (acceleration > 0.001f) {
        acceleration = std::min(acceleration, 1.0f);
        const float speed = std::sqrt(dx * dx + dy * dy);
        const float factor = 1.0f + acceleration * std::min(speed / 60.0f, 3.0f);
        dx *= factor;
        dy *= factor;
    }
}

void ResetMouseCurve(float& smoothX, float& smoothY) { smoothX = smoothY = 0.0f; }

int SelfTest() {
    std::ofstream report("gametype-selftest.log", std::ios::app);
    auto fail = [&report](const char* what) {
        std::printf("FAIL  InputTuning: %s\n", what);
        if (report) report << "FAIL  InputTuning: " << what << "\n";
    };
    int failures = 0;
    auto expect = [&](bool ok, const char* what) {
        if (!ok) { fail(what); ++failures; }
    };

    // --- Dead zone: off by default, kills everything inside the zone, keeps full speed at the edge.
    {
        int16_t x = 12000, y = -9000;
        ApplyDeadZone(x, y, 0.0f);
        expect(x == 12000 && y == -9000, "zone 0 must not change the stick");
    }
    {
        int16_t x = 1000, y = 800;   // length ~0.04, inside a 0.2 zone
        ApplyDeadZone(x, y, 0.2f);
        expect(x == 0 && y == 0, "input inside the dead zone must become 0");
    }
    {
        int16_t x = 32767, y = 0;
        ApplyDeadZone(x, y, 0.2f);
        expect(x > 32000, "the edge of the dead zone must still reach full speed");
    }
    {
        int16_t a = 20000, b = 0;
        ApplyDeadZone(a, b, 0.1f);
        int16_t c = 26000, d = 0;
        ApplyDeadZone(c, d, 0.1f);
        expect(c > a, "a bigger flick must come out bigger after the dead zone");
    }

    // --- Stick layout: Southpaw swaps, Default does not.
    {
        int16_t lx = 1000, ly = 2000, rx = 3000, ry = 4000;
        ApplyStickLayout(0, lx, ly, rx, ry);
        expect(lx == 1000 && rx == 3000, "Default stick layout must not swap");
        ApplyStickLayout(1, lx, ly, rx, ry);
        expect(lx == 3000 && ly == 4000 && rx == 1000 && ry == 2000, "Southpaw must swap the two sticks");
    }

    // --- Mouse curve: both parts disabled by default.
    {
        float dx = 25.0f, dy = -10.0f, sx = 0.0f, sy = 0.0f;
        ApplyMouseCurve(dx, dy, 0.0f, 0.0f, sx, sy);
        expect(dx == 25.0f && dy == -10.0f, "acceleration and smoothing off must pass the delta through");
    }
    // Acceleration makes a fast flick grow more than a slow one, and never unbounded.
    {
        float slowX = 10.0f, slowY = 0.0f, s1 = 0.0f, s2 = 0.0f;
        ApplyMouseCurve(slowX, slowY, 1.0f, 0.0f, s1, s2);
        float fastX = 200.0f, fastY = 0.0f, s3 = 0.0f, s4 = 0.0f;
        ApplyMouseCurve(fastX, fastY, 1.0f, 0.0f, s3, s4);
        const float slowGain = slowX / 10.0f;
        const float fastGain = fastX / 200.0f;
        expect(fastGain > slowGain, "acceleration must help fast flicks more than slow movement");
        expect(fastGain <= 4.0f, "acceleration must stay capped");
        float plainX = 200.0f, plainY = 0.0f, s5 = 0.0f, s6 = 0.0f;
        ApplyMouseCurve(plainX, plainY, 0.0f, 0.0f, s5, s6);
        expect(fastX > plainX, "acceleration on must turn further than off for the same flick");
    }
    // Smoothing eases toward the input instead of jumping to it, and keeps doing so.
    {
        float sx = 0.0f, sy = 0.0f;
        float firstX = 100.0f, firstY = 0.0f;
        ApplyMouseCurve(firstX, firstY, 0.0f, 0.8f, sx, sy);
        expect(firstX < 50.0f, "smoothing must damp the first frame of a flick");
        float secondX = 100.0f, secondY = 0.0f;
        ApplyMouseCurve(secondX, secondY, 0.0f, 0.8f, sx, sy);
        expect(secondX > firstX, "smoothing must keep moving toward the input");
        ResetMouseCurve(sx, sy);
        expect(sx == 0.0f && sy == 0.0f, "ResetMouseCurve must clear the filter state");
    }

    std::printf("%s: input tuning - %d failure(s)\n", failures == 0 ? "PASS" : "FAIL", failures);
    if (report) report << (failures == 0 ? "PASS" : "FAIL") << ": input tuning - " << failures << " failure(s)\n";
    return failures;
}

} // namespace InputTuning
} // namespace noble
