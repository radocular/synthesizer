#if !defined(SEQUENCER)
#define SEQUENCER

#if defined(__linux__)
#include <alsa/asoundlib.h>
#endif

#include "instrument.cpp"
#include "waveform.cpp"
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono;

class Sequencer {
public:
    Sequencer() { init(); }

    void loop(Organ &organ) {
        _taskMIDI =
            thread([this](Organ &organ) { loopMIDI(organ); }, ref(organ));
        _taskMIDI.detach();

        /*_taskADSR =
            thread([this](Organ &organ) { loopADSR(organ); }, ref(organ));
        _taskADSR.detach();*/
    }

private:
    void loopADSR(Organ &organ) {
        while (true) {
            for (int64_t i = 0; i < 49; ++i) {
            }
        }
    }

    void loopMIDI(Organ &organ) {
        while (true) {
            snd_seq_event_input(_handle, &_event);
            int64_t note = _event->data.note.note;
            int64_t velocity = _event->data.note.velocity;

            if (_event->type == SND_SEQ_EVENT_NOTEON) {
                if (velocity == 0) {
                    organ.setVolume(note, 0);
                    // organ.stop(note);
                } else {
                    // organ.play(note);
                    organ.setVolume(note, 100.0L * (velocity / 127.0L));
                }
            }
        }
    }

    snd_seq_t *_handle;
    int64_t _port;
    snd_seq_event_t *_event;
    thread _taskMIDI;
    thread _taskADSR;

    void init() {
        snd_seq_open(&_handle, "default", SND_SEQ_OPEN_INPUT, 0);
        snd_seq_set_client_name(_handle, "Keyboard Listener");
        _port = snd_seq_create_simple_port(
            _handle, "in", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION);

        snd_seq_addr_t sender, dest;
        snd_seq_port_subscribe_t *subs;
        sender.client = 24;
        sender.port = 1;
        dest.client = 130;
        dest.port = 0;
        snd_seq_port_subscribe_alloca(&subs);
        snd_seq_port_subscribe_set_sender(subs, &sender);
        snd_seq_port_subscribe_set_dest(subs, &dest);
        snd_seq_port_subscribe_set_queue(subs, 1);
        snd_seq_port_subscribe_set_time_update(subs, 1);
        snd_seq_port_subscribe_set_time_real(subs, 1);
        snd_seq_subscribe_port(_handle, subs);
    }
};

#endif
