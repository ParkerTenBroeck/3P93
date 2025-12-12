#ifndef GUI

#include <chrono>

#include <args.h>
#include <game.h>
#include <ui/tui.h>

void run_tui(const Arguments& args){

    auto game = Game::make_game(args);

    const f64 total_duration = args.frames/args.framerate;
    const u64 frames = args.frames;

    const auto start = std::chrono::high_resolution_clock::now();
    for (u64 i = 0; i < frames; i ++) {
        auto frame_start = std::chrono::high_resolution_clock::now();
        game->update(1.f/total_duration, i*total_duration/args.framerate);
        game->render();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - frame_start;
        long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::cout << "Frame: " << (i+1) << " Render Time: " << milliseconds << " ms" << std::endl;

        if (args.write_frames)
            write_image(*game, "../animation/frame_" + leading(i, 3) + ".png");
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration<f64>(end - start).count();
    std::cerr << "" << duration << "" << std::endl;
}

#endif