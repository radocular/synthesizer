#include "frontend.cpp"
#include "instrument.cpp"
#include "sequencer.cpp"

using namespace sf;
using namespace std;
using namespace synth;

int main() {
    static auto frontend = Frontend();
    static auto organ = Organ(frontend.getWavetableBuilder());
    static auto sequencer = Sequencer();

    sequencer.loop(organ);
    frontend.loop(organ);

    return 0;
}
