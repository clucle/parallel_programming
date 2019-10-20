#include "worker.hpp"

Worker::Worker(int tid, std::unique_ptr<Database> &db) : tid(tid), db(db)
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
    db->acquire();
    std::cout << "tid : " << tid << '\n';
    db->release();
}

void Worker::set_i_j_k()
{
}
