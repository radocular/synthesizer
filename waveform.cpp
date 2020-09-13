#if !defined(WAVEFORM)
#define WAVEFORM

#include <SFML/Audio.hpp>
#include <cmath>
#include <limits>

#if defined(__linux__)
#include <sys/random.h>
#endif

namespace synth {
    using namespace std;

    using int_osc_t = int16_t;
    using float128_t = long double;

    using ApplyWaveform = function<float128_t(const int64_t, const float128_t)>;

    using BuildWavetable =
        function<void(vector<int_osc_t> &, const float128_t, const float128_t)>;

    constexpr float128_t pi = 0x3.243f6a8885a308d313198a2e03707344p0L;
    constexpr float128_t piDivTwo = pi / 2.0L;
    constexpr float128_t twoDivPi = 2.0L / pi;
    constexpr float128_t twoPi = 2.0L * pi;
    constexpr float128_t fourPi = 4.0L * pi;

    constexpr float128_t maxAmp = numeric_limits<int_osc_t>::max();
    constexpr float128_t stdRate = 8388608.0L; // 262144; // 1048576; // 8388608.0L;
    constexpr float128_t stdFreq = 440.0L;

    constexpr float128_t freq2TPC(const float128_t freq,
                                  const float128_t rate = stdRate) {
        return rate / freq;
    }

    constexpr float128_t freq2Cycle(const float128_t freq) {
        return 1.0L / (freq2TPC(freq) - 1.0L);
    }

    constexpr float128_t timeByFreq2Cycle(const int64_t time,
                                          const float128_t freq) {
        return time / (freq2TPC(freq) - 1.0L);
    }

    ApplyWaveform scaleWaveformAt(const float128_t division,
                                  ApplyWaveform apply) {
        const int64_t time = round(division * (freq2TPC(1.0L) - 1.0L));
        const float128_t scale = 1.0L / apply(time, 1.0L);

        return [scale, apply](int64_t time, float128_t freq) {
            return scale * apply(time, freq);
        };
    }

    ApplyWaveform shiftWaveformAt(const float128_t division,
                                  ApplyWaveform apply) {
        const int64_t time = round(division * (freq2TPC(1.0L) - 1.0L));
        const float128_t shift = apply(time, 1.0L);

        return [apply, shift](const int64_t time, const float128_t freq) {
            return apply(time, freq) - shift;
        };
    }

    ApplyWaveform expandWaveform(const float128_t factor, ApplyWaveform apply) {
        return [factor, apply](const int64_t time, const float128_t freq) {
            return factor * apply(time, freq) - (factor * 0.5);
        };
    }

    ApplyWaveform reverseWaveform(ApplyWaveform apply) {
        return [apply](const int64_t time, const float128_t freq) {
            return -apply(time, freq);
        };
    }

    ApplyWaveform identityWaveform(ApplyWaveform apply) { return apply; }

    constexpr float128_t sineWaveform(const int64_t time,
                                      const float128_t freq) {
        return sin(twoPi * timeByFreq2Cycle(time, freq));
    }

    constexpr float128_t triangleWaveform(const int64_t time,
                                          const float128_t freq) {
        return twoDivPi * asin(sin(twoPi * timeByFreq2Cycle(time, freq)));
    }

    ApplyWaveform triangleWaveformFourier(const int64_t approx) {
        auto apply = [approx](const int64_t time, const float128_t freq) {
            float128_t sum = 0.0L;
            float128_t sign = -1.0L;

            for (int64_t i = 1L; i <= approx; ++i) {
                sum +=
                    sign *
                    sin((fourPi * i - twoPi) * timeByFreq2Cycle(time, freq)) /
                    ((2.0L * i - 1.0L) * (2.0L * i - 1.0L));
                sign = -sign;
            }

            return -sum;
        };

        return scaleWaveformAt(0.25L, apply);
    }

    ApplyWaveform squareWaveform(const float128_t division) {
        return [division](const int64_t time, const float128_t freq) {
            const float128_t tpc = freq2TPC(freq);

            return fmod(time, tpc + 1.0L) < (division * tpc) ? 1.0L : -1.0L;
        };
    }

    ApplyWaveform squareWaveformFourier(const int64_t approx) {
        auto apply = [approx](const int64_t time, const float128_t freq) {
            float128_t sum = 0.0L;

            for (int64_t i = 1L; i <= 2L * approx; i += 2L)
                sum += sin(i * twoPi * timeByFreq2Cycle(time, freq)) / i;

            return sum;
        };

        return scaleWaveformAt(1.0L / (4.0L * approx), apply);
    }

    constexpr float128_t sawtoothWaveform(const int64_t time,
                                          const float128_t freq) {
        return -twoDivPi *
               (freq2Cycle(freq) * pi * fmod(time, freq2TPC(freq)) - piDivTwo);
    }

    ApplyWaveform sawtoothWaveformFourier(const int64_t approx) {
        auto apply = [approx](const int64_t time, const float128_t freq) {
            float128_t sum = 0.0L;

            for (int64_t i = 1L; i <= approx; ++i)
                sum += sin(i * twoPi * timeByFreq2Cycle(time, freq)) / i;

            return sum;
        };

        return scaleWaveformAt(1.0L / (2.0L + 2.0 * approx), apply);
    }

    constexpr float128_t sinePulseWaveform(const int64_t time,
                                           const float128_t freq) {
        return 2.0L * abs(sin(pi * timeByFreq2Cycle(time, freq))) - 1.0;
    }

    ApplyWaveform sinePulseWaveformFourier(const int64_t approx) {
        auto apply = [approx](const int64_t time, const float128_t freq) {
            float128_t sum = 0.0L;

            for (int64_t i = 1L; i <= approx; ++i)
                sum += cos(i * twoPi * timeByFreq2Cycle(time, freq)) /
                       (4.0L * i * i - 1.0L);

            return -sum;
        };

        return expandWaveform(
            2.0L, scaleWaveformAt(0.5L, shiftWaveformAt(0L, apply)));
    }

    float128_t randomNoise(const int64_t time, const float128_t freq) {
        if constexpr (__linux__) {
            int_osc_t buffer;
            getrandom(&buffer, sizeof(int_osc_t), GRND_RANDOM);
            return buffer / maxAmp;
        } else {
            return 0;
        }
    }

    constexpr float128_t maxWaveform(const int64_t time,
                                     const float128_t freq) {
        return 1.0L;
    }

    constexpr float128_t ecuadorWaveform(const int64_t time,
                                         const float128_t freq) {
        return 0.0L;
    }

    constexpr float128_t minWaveform(const int64_t time,
                                     const float128_t freq) {
        return -1.0L;
    }

    BuildWavetable variadic(ApplyWaveform apply) {
        return [apply](vector<int_osc_t> &table, const float128_t freq,
                       const float128_t amp) {
            const int64_t tpc = table.size();

            for (int64_t time = 0L; time < tpc; ++time)
                table[time] = round(amp * apply(time, freq));
        };
    }

    BuildWavetable axisymmetric(ApplyWaveform apply) {
        return [apply](vector<int_osc_t> &table, const float128_t freq,
                       const float128_t amp) {
            const int64_t tpc = table.size();
            const int64_t half = tpc / 2L;

            for (int64_t time = 0L; time <= half; ++time)
                table[time] = round(amp * apply(time, freq));

            for (int64_t time = 1L; time <= half; ++time)
                table[tpc - time] = table[time];
        };
    }

    BuildWavetable pointsymmetric(ApplyWaveform apply) {
        return [apply](vector<int_osc_t> &table, const float128_t freq,
                       const float128_t amp) {
            const int64_t tpc = table.size();
            const int64_t half = tpc / 2L;

            for (int64_t time = 0L; time <= half; ++time)
                table[time] = round(amp * apply(time, freq));

            for (int64_t time = 1; time <= half; ++time)
                table[tpc - time] = -table[time];
        };
    }

    BuildWavetable periodic(ApplyWaveform apply) {
        return [apply](vector<int_osc_t> &table, const float128_t freq,
                       const float128_t amp) {
            const int64_t tpc = table.size();
            const int64_t half = tpc / 2L;
            const int64_t quart = tpc / 4L;

            for (int64_t time = 0L; time <= quart; ++time)
                table[time] = round(amp * apply(time, freq));

            for (int64_t time = 0L; time <= quart; ++time)
                table[half - time] = table[time];

            for (int64_t time = 1L; time <= half + 1; ++time)
                table[tpc - time] = -table[time];
        };
    }
}

#endif
