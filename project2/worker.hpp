#ifndef C_WORKER
#define C_WORKER

#include <thread>
#include <iostream>

class Worker
{
public:
    Worker(int tid);
    void run_thread();
    void join_thread();

private:
    int tid;
    int i, j, k;
    std::thread t;

    void work();
    void set_i_j_k();
};

#endif
