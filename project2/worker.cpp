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
    std::mutex &g_mutex = db->get_lock();
    std::unique_lock<std::mutex> g_lk(g_mutex, std::defer_lock);
    std::unique_ptr<std::condition_variable> cv(new std::condition_variable);

    set_i_j_k();
    g_lk.lock();
    std::cout << "tid : " << tid << ' ' << i << ' ' << j << ' ' << k << '\n';
    ERecordLockState state = db->rd_lock(i, 3, cv);
    if (state == ERecordLockState::EWAIT)
    {
        cv->wait(g_lk);
    }
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
