

#include <args.h>

#include <mpi.h>

#ifdef GUI
#include <ui/gui.h>
#else
#include <ui/tui.h>
#endif


#ifdef USE_OPEN_MPI

void mpi_master() {
    printf("Master started\n");
    usize frame = 0;

    while (true) {
        auto receiver = mpi::Receiver::begin();

        switch (receiver.type()) {
            case mpi::TypeTag::S2M_Ready:
                receiver.sender().send((++frame)/60.0);
                break;
            case mpi::TypeTag::S2M_FrameComplete:
                break;
            case mpi::TypeTag::M2S_BeginFrame:
                break;
            case mpi::TypeTag::M2S_LoadFile: {
                auto path = receiver.receive<std::string>();
                auto data = file_load_string(path);
                receiver.sender().send(data);
            }break;
            default:
                break;
        }
    }
}
#include <chrono>
#include <thread>
void mpi_slave() {
    Scene scene;
    ResourceStore resource_store;
    resource_store.normal_map("../assets/brick/normal_test.png");

    while (true) {
        auto received = mpi::s::slave_await_begin_frame();
        printf("Received %lf frames\n", received);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    if (mpi::is_master()) {
        mpi_master();
    } else {
        mpi_slave();
    }

    MPI_Finalize();
    return 0;
}

#else

int main(int argc, char** argv) {
    Arguments args(argv, argc);

    #ifdef GUI
    run_gui(args);
    #else
    run_tui(args);
    #endif
}

#endif
