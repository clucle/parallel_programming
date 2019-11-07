#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

#include "worker.hpp"
#include "snapshot.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage : ./run N\n";
        return 0;
    }

    int N = atoi(argv[1]);

    std::unique_ptr<WaitFreeSnapshot> snapshot(new WaitFreeSnapshot(N));
    std::vector<Worker> v_worker;

    for (int tid = 0; tid < N; tid++)
    {
        v_worker.emplace_back(tid, snapshot);
    }

    for (int i = 0; i < N; i++)
    {
        v_worker[i].run_thread();
    }

    std::this_thread::sleep_for(std::chrono::seconds(60));

    for (int i = 0; i < N; i++)
    {
        v_worker[i].join_thread();
    }

    std::cout << "Update Cnt : "<< snapshot->get_cnt() << '\n';

    return 0;
}
