

#include <args.h>
#include <mpi/mpi.h>
#include <util/fs.h>

#include "game.h"

#ifdef GUI
#include <ui/gui.h>
#else
#include <ui/tui.h>
#endif


#ifdef USE_OPEN_MPI

void mpi_coordinator(int argc, char** argv) {
    Arguments args(argv, argc);
    mpi::coordinator_broadcast(args, mpi::Tag::Arguments);

    auto frame_buffer = new Vector4<u8>[args.height*args.width];

    auto start = std::chrono::high_resolution_clock::now();

    u32 frame = 0;
    u32 outstanding = mpi::worker_count();
    while (frame < args.frames || outstanding!=0) {
        auto receiver = mpi::Receiver::probe();

        switch (receiver.type()) {
            case mpi::Tag::FrameComplete: {
                const auto finished_frame = receiver.receive<u32>();
                if (finished_frame == 0xFFFFFFFF) {
                    outstanding--;
                }else if (args.write_frames) {
                    receiver.receive(frame_buffer, args.height*args.width);
                    auto path = "../animation/frame_" + leading(finished_frame, 3) + ".png";
                    stbi_write_png(path.c_str(), args.width, args.height, 4, frame_buffer, args.width * 4);
                }
            }
            break;
            case mpi::Tag::WorkerReady: {
                if (frame == 0) start = std::chrono::high_resolution_clock::now();

                if (frame < args.frames) {
                    receiver.receive<mpi::Empty>(nullptr).sender(mpi::Tag::BeginFrame).send(frame);
                    frame++;
                }else {
                    receiver.receive<mpi::Empty>(nullptr).sender(mpi::Tag::BeginFrame).send(0xFFFFFFFF);
                }
            }
            break;
            case mpi::Tag::LoadFile: {
                auto path = receiver.receive<std::string>();
                auto data = file_load_string(path);
                receiver.sender().send(data);
            }break;

            default:
                break;
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration<f64>(end - start).count();
    std::cerr << "" << duration << "" << std::endl;
}

void mpi_worker() {
    const auto args = mpi::Receiver::begin(mpi::COORDINATOR, mpi::Tag::Arguments).receive<Arguments>();
    auto game = Game::make_game(args);

    auto frame_buffer = new Vector4<u8>[args.height*args.width];

    f64 frame_start = 0;
    while (true) {
        auto receiver = mpi::Sender::begin(mpi::COORDINATOR, mpi::Tag::WorkerReady).send(mpi::Empty{}).receiver(mpi::Tag::BeginFrame);
        u32 frame = receiver.receive<u32>();
        if (frame == 0xFFFFFFFF) {
            receiver.sender(mpi::Tag::FrameComplete).send(frame);
            return;
        }
        const f64 time = frame / args.framerate;

        auto start = std::chrono::high_resolution_clock::now();

        game->update((f32)(time-frame_start), time);
        frame_start = time;
        game->render();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::cout << "Worker: " << mpi::rank() << " Finished Frame: " << (frame+1) << " Render Time: " << milliseconds << " ms" << std::endl;

        if (args.write_frames) {
            #ifdef USE_OPEN_MP
            #pragma omp parallel for
            #endif
            for (usize i = 0; i < args.height * args.width; i++) {
                auto color = game->frame_buffer[i].diffuse;
                auto normal = game->frame_buffer[i].normal;

                if (normal.magnitude_squared() != 0) {
                    frame_buffer[i].x() = static_cast<u8>(std::min(255.f, std::pow(color.x(), 1.f/2.2f) * 255));
                    frame_buffer[i].y() = static_cast<u8>(std::min(255.f, std::pow(color.y(), 1.f/2.2f) * 255));
                    frame_buffer[i].z() = static_cast<u8>(std::min(255.f, std::pow(color.z(), 1.f/2.2f) * 255));
                    frame_buffer[i].w() = 255;
                } else {
                    frame_buffer[i].w() = 0;
                }
            }

            receiver.sender(mpi::Tag::FrameComplete)
                .send(frame)
                .send(frame_buffer, args.height*args.width);
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    if (mpi::is_coordinator()) {
        mpi_coordinator(argc, argv);
    } else {
        mpi_worker();
    }
    std::cout << mpi::rank() << " finished" << std::endl;

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
