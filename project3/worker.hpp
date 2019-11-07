#ifndef C_WORKER
#define C_WORKER

#include <thread>
#include <memory>
#include <random>
#include <chrono>
#include <climits>

#include "snapshot.hpp"

class Worker
{
public:
    Worker(int tid, std::unique_ptr<WaitFreeSnapshot> &snapshot);
    void run_thread();
    void join_thread();

private:
    int tid;
    std::thread t;
    std::unique_ptr<WaitFreeSnapshot> &snapshot;
    bool is_running;
    void work();
    int get_random_num();

    // Test
    int seq;
    int get_sequential_num();
};
#endif