#include "worker.hpp"

Worker::Worker(int tid, std::unique_ptr<WaitFreeSnapshot> &snapshot)
    : tid(tid), snapshot(snapshot)
{
    seq = 0;
    is_running = false;
}

void Worker::run_thread()
{
    is_running = true;
    t = std::thread(&Worker::work, this);
}

void Worker::join_thread()
{
    is_running = false;
    t.join();
}

void Worker::work()
{
    while (is_running)
    {
        //*
        int value = get_random_num();
        /*/
        int value = get_sequential_num();
        //*/
        snapshot->update(tid, value);
    }
}

int Worker::get_random_num()
{
    auto gen = std::mt19937{std::random_device{}()};
    auto dist = std::uniform_int_distribution<int>{INT_MIN, INT_MAX};
    return dist(gen);
}

int Worker::get_sequential_num()
{
    return ++seq;
}
