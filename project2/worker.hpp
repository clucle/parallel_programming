#ifndef C_WORKER
#define C_WORKER

#include <thread>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <condition_variable>
#include "database.hpp"

class Worker
{
public:
    Worker(int tid, std::unique_ptr<Database> &db);
    void run_thread();
    void join_thread();

private:
    int tid;
    int i, j, k;
    std::thread t;
    std::unique_ptr<Database> &db;

    void work();
    void set_i_j_k();
};

#endif
