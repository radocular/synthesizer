#if !defined(INSTRUMENT)
#define INSTRUMENT

#include <SFML/Audio.hpp>
#include "waveform.cpp"

using namespace sf;
using namespace std;
using namespace synth;

class Organ {
public:
    Organ(BuildWavetable build) {
        for (int64_t i = 0; i < scale; ++i) {
            table[i] = vector<int_osc_t>(round(freq2TPC(temperament[i])));
        }
        
        create(build);
    }

    void create(BuildWavetable build) {
        for (int64_t i = 0; i < scale; ++i)
            build(table[i], temperament[i], maxAmp);

        for (int64_t i = 0; i < scale; ++i)
            buffer[i].loadFromSamples(&table[i][0], table[i].size(), 1,
                                      stdRate);

        for (int64_t i = 0; i < scale; ++i) {
            sound[i].setBuffer(buffer[i]);
            sound[i].setLoop(true);
            sound[i].setVolume(0);
            sound[i].play();
        }
    }

    const vector<int_osc_t> &getWavetable(int64_t note) {
        return table[note + shift];
    }

    void play(int64_t note) { sound[note + shift].play(); }

    void stop(int64_t note) { sound[note + shift].stop(); }

    bool isActive(int64_t note) {
        return sound[note + shift].getStatus() == SoundSource::Status::Playing;
    }

    void setVolume(int64_t note, float128_t volume) {
        sound[note + shift].setVolume(volume);
    }

    consteval static int64_t getShift() { return shift; }

private:
    constexpr static int64_t shift = -36;
    constexpr static int64_t scale = 49L;

    constexpr static float128_t temperament[scale] = {
        65.4064, 69.2957, 73.4162, 77.7817, 82.4069, 87.3071, 92.4986,
        97.9989, 103.826, 110.000, 116.541, 123.471, 130.813, 138.591,
        146.832, 155.563, 164.814, 174.614, 184.997, 195.998, 207.652,
        220.000, 233.082, 246.942, 261.626, 277.183, 293.665, 311.127,
        329.628, 349.228, 369.994, 391.995, 415.305, 440.000, 466.164,
        493.883, 523.251, 554.365, 587.330, 622.254, 659.255, 698.456,
        739.989, 783.991, 830.609, 880.000, 932.328, 987.767, 1046.50
    };

    vector<int_osc_t> table[scale];

    SoundBuffer buffer[scale];

    Sound sound[scale];
};

class Pipe {
public:
    Pipe(BuildWavetable build) { create(build); }

    void create(BuildWavetable build) {
        build(table, stdFreq, maxAmp);
        buffer.loadFromSamples(&table[0], table.size(), 1, stdRate);
        sound.setBuffer(buffer);
        sound.setLoop(true);
    }

    const vector<int_osc_t> &getWavetable() { return table; }

    void play() { sound.play(); }

    void stop() { sound.stop(); }

    bool isActive() {
        return sound.getStatus() == SoundSource::Status::Playing;
    }

    void setVolume(int64_t note, float128_t volume) { sound.setVolume(volume); }

private:
    vector<int_osc_t> table = vector<int_osc_t>(round(freq2TPC(stdFreq)));

    SoundBuffer buffer = SoundBuffer();

    Sound sound = Sound();
};

#endif
