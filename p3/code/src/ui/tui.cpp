#ifndef GUI

#include <chrono>

#include <args.h>
#include <game.h>
#include <ui/tui.h>

void run_tui(const Arguments& args){

    auto game = Game::make_game(args);

    f64 total_duration = 3.;
    u64 frames = 300;
    u64 total_ms = 0;
    for (u64 i = 0; i < frames; i ++) {
        auto start = std::chrono::high_resolution_clock::now();
        game->update(1.f/total_duration, i*total_duration/frames);
        game->render();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        total_ms += milliseconds;
        std::cout << "Frame: " << (i+1) << " Render Time: " << milliseconds << " ms" << std::endl;

        if (args.write_frames)
            write_image(*game, "../animation/frame_" + leading(i, 3) + ".png");
    }
    std::cout << "average frame time: " << (total_ms/300.0) << "ms" << std::endl;
}

#endif