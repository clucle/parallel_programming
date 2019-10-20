#include <iostream>
#include <cstdlib>
#include <vector>

#include "worker.hpp"

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        std::cerr << "Usage : ./run N R E\n";
        return 0;
    }

    int N = atoi(argv[1]);
    int R = atoi(argv[2]);
    int E = atoi(argv[3]);

    if (R < 3)
    {
        std::cerr << "number of records is greater or equal than 3\n";
        return 0;
    }

    std::vector<Worker> v_worker;
    for (int i = 0; i < N; i++) {
        v_worker.emplace_back(i + 1);
    }

    for (int i = 0; i < N; i++) {
        v_worker[i].run_thread();
    }

    for (int i = 0; i < N; i++) {
        v_worker[i].join_thread();
    }
}
