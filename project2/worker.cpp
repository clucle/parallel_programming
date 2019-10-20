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
    std::mutex &g_lk = db->get_lock();
    set_i_j_k();
    g_lk.lock();
    std::cout << "tid : " << tid << ' ' << i << ' ' << j << ' ' << k << '\n';
    g_lk.unlock();
}

void Worker::set_i_j_k()
{
    int max_val = db->size();
    std::vector<int> v;
    for (int i = 1; i <= max_val; i++)
        v.push_back(i);
    std::random_device rd;
    std::mt19937 mt(rd());
    std::shuffle(v.begin(), v.end(), mt);
    i = v[0];
    j = v[1];
    k = v[2];
}
