#include "worker.hpp"

Worker::Worker(int tid) : tid(tid)
{
}
void Worker::run_thread()
{
    t = std::thread(&Worker::work, this);
}
void Worker::join_thread()
{
    t.join();
}

void Worker::work()
{
    std::cout << "tid : " << tid << '\n';
}

void Worker::set_i_j_k()
{
}
