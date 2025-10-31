

#include <args.h>

#ifdef GUI
#include <ui/gui.h>
#else
#include <ui/tui.h>
#endif

int main(int argc, char** argv) {
    Arguments args(argv, argc);

    #ifdef GUI
    run_gui(args);
    #else
    run_tui(args);
    #endif
}
