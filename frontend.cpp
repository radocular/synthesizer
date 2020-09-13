#if !defined(FRONTEND)
#define FRONTEND

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <vector>

#include "instrument.cpp"
#include "waveform.cpp"

using namespace sf;
using namespace std;
using namespace synth;

class Frontend {
public:
    Frontend() {
        _icon.loadFromFile("./icon.png");
        _window.setIcon(_icon.getSize().x, _icon.getSize().y,
                        _icon.getPixelsPtr());
        _body(_pipe.getWavetable());
    }

    BuildWavetable getWavetableBuilder() {
        auto transform = _reverse ? reverseWaveform : identityWaveform;

        switch (_select) {
            case 0:
                return variadic(transform(sineWaveform));
            case 1:
                return variadic(transform(triangleWaveform));
            case 2:
                return variadic(transform(triangleWaveformFourier(_approx)));
            case 3:
                return variadic(transform(squareWaveform(_division)));
            case 4:
                return variadic(transform(squareWaveformFourier(_approx)));
            case 5:
                return variadic(transform(sawtoothWaveform));
            case 6:
                return variadic(transform(sawtoothWaveformFourier(_approx)));
            case 7:
                return variadic(transform(sinePulseWaveform));
            case 8:
                return variadic(transform(sinePulseWaveformFourier(_approx)));
            default:
                return variadic(transform(randomNoise));
        }
    }

    void loop(Organ &organ) {
        auto event = Event();

        while (_window.isOpen()) {
            while (_window.pollEvent(event)) {
                if (event.type == Event::KeyPressed and
                    Keyboard::isKeyPressed(Keyboard::Up)) {
                    _approx += 1L;
                    _update();
                }

                else if (event.type == Event::KeyPressed and
                         Keyboard::isKeyPressed(Keyboard::Down)) {
                    if (_approx > 1)
                        _approx -= 1;

                    _update();
                }

                else if (event.type == Event::Closed)
                    _window.close();

                else if (event.type == Event::KeyPressed and
                         Keyboard::isKeyPressed(Keyboard::R)) {
                    _reverse = !_reverse;
                    _update();
                }

                else if (event.type == Event::KeyPressed and
                         Keyboard::isKeyPressed(Keyboard::Space)) {
                    if (!_pipe.isActive())
                        _pipe.play();
                }

                else if (event.type == Event::KeyReleased and
                         !Keyboard::isKeyPressed(Keyboard::Space)) {
                    _pipe.stop();
                }

                else if (event.type == Event::KeyPressed and
                         Keyboard::isKeyPressed(Keyboard::Left)) {
                    _select = _select > 1 ? _select - 1L : _choice - 1;
                    _update();
                }

                else if (event.type == Event::KeyPressed and
                         Keyboard::isKeyPressed(Keyboard::Right)) {
                    _select = (_select += 1L) % _choice;
                    _update();
                }

                else if (event.type == Event::KeyPressed and
                         Keyboard::isKeyPressed(Keyboard::Enter))
                    organ.create(getWavetableBuilder());

                else if (event.type == Event::KeyPressed and
                         Keyboard::isKeyPressed(Keyboard::A)) {
                    if (_division > 0.0L)
                        _division -= (1.0L / 128.0L);

                    _update();
                }

                else if (event.type == Event::KeyPressed and
                         Keyboard::isKeyPressed(Keyboard::B)) {
                    if (_division > 0.0L)
                        _division += (1.0L / 128.0L);

                    _update();
                }
            }

            _window.clear();
            _window.draw(&_chart[0], size_t(_width), LineStrip);
            _window.display();
        }
    }

private:
    void _update() {
        _pipe.create(getWavetableBuilder());
        _head();
        _body(_pipe.getWavetable());
    }

    constexpr static float128_t _repeat = 2.0L;
    constexpr static float128_t _cycle = 600.0L;
    constexpr static float128_t _width = _repeat * _cycle;
    constexpr static float128_t _height = 600.0L;
    constexpr static float128_t _screen = _height / 3.0L;
    constexpr static int64_t _choice = 10;

    Pipe _pipe = Pipe(getWavetableBuilder());

    RenderWindow _window = RenderWindow(VideoMode(1200, 600), "Synth - Sine");

    Image _icon;

    Vertex _chart[size_t(_width)];

    int64_t _approx = 1L;

    float128_t _division = 0.5L;

    bool _reverse = false;

    int64_t _select = 0L;

    void _body(const vector<int_osc_t> &table) {
        for (int64_t i = 0; i < _repeat; ++i) {
            for (int64_t xAxis = i * _cycle; xAxis < (1.0L + i) * _cycle;
                 ++xAxis) {
                float128_t yAxis =
                    ((xAxis - i * _cycle) / _cycle) * freq2TPC(stdFreq);
                _chart[xAxis] = Vertex(
                    Vector2f(xAxis, -table[round(yAxis)] / (maxAmp / _screen) +
                                        1.5L * _screen));
            }
        }
    }

    void _head() {
        switch (_select) {
            case 0:
                _window.setTitle("Synth - Sine");
                break;
            case 1:
                _window.setTitle("Synth - Triangle");
                break;
            case 2:
                _window.setTitle("Synth - Triangle Fourier (approx = " +
                                 to_string(_approx) + ")");
                break;
            case 3:
                _window.setTitle(
                    "Synth - Square (division = " + to_string(_division) + ")");
                break;
            case 4:
                _window.setTitle("Synth - Square Fourier (approx = " +
                                 to_string(_approx) + ")");
                break;
            case 5:
                _window.setTitle("Synth - Sawtooth");
                break;
            case 6:
                _window.setTitle("Synth - Sawtooth Fourier (approx = " +
                                 to_string(_approx) + ")");
                break;
            case 7:
                _window.setTitle("Synth - Sine Pulse");
                break;
            case 8:
                _window.setTitle("Synth - Sine Pulse Fourier (approx = " +
                                 to_string(_approx) + L")");
                break;
            default:
                _window.setTitle("Synth - Random Noise");
        }
    }
};

#endif
