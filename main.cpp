#include "snake.h"

int main(int argc, char *argv[]) {
    // Start input handler in its own thread
    thread input_thread(input_handler);

    // Run game loop (handles replay + leaderboard)
    run_game();

    // Wait for input thread before exiting
    input_thread.join();

    return 0;
}
