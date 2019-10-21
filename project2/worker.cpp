#include "worker.hpp"

#define TEST_DEADLOCK true

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
    ERecordLockState r_lk_state;
    long long record_i, record_j, record_k;

    while (true)
    {
        set_i_j_k();
        g_lk.lock();
        std::cout << "tid : " << tid << ' ' << i << ' ' << j << ' ' << k << '\n';
        r_lk_state = db->rd_lock(i, tid, cv);
        if (r_lk_state == ERecordLockState::EWAIT)
        {
            std::cout << "tid : " << tid << " record : " << i << " read wait\n";
            cv->wait(g_lk);
        }
        std::cout << "tid : " << tid << " record : " << i << " read acquire\n";
        g_lk.unlock();
        record_i = db->get_record(i);

        if (TEST_DEADLOCK)
            std::this_thread::yield();

        g_lk.lock();
        r_lk_state = db->wr_lock(j, tid, cv);
        if (r_lk_state == ERecordLockState::EDEADLOCK)
        {
            std::cout << "tid : " << tid << " record : " << j << " write1 deadlock\n";
            db->rw_unlock(i, tid);
            g_lk.unlock();
            continue;
        }
        else if (r_lk_state == ERecordLockState::EWAIT)
        {
            std::cout << "tid : " << tid << " record : " << j << " write1 wait\n";
            cv->wait(g_lk);
        }
        std::cout << "tid : " << tid << " record : " << j << " write1 acquire\n";
        g_lk.unlock();
        record_j = db->get_record(j);
        record_j += record_i + 1;
        db->set_record(j, record_j);

        if (TEST_DEADLOCK)
            std::this_thread::yield();

        g_lk.lock();
        r_lk_state = db->wr_lock(k, tid, cv);
        if (r_lk_state == ERecordLockState::EDEADLOCK)
        {
            std::cout << "tid : " << tid << " record : " << k << " write2 deadlock\n";
            record_j -= record_i + 1;
            db->set_record(j, record_j);
            db->rw_unlock(i, tid);
            db->rw_unlock(j, tid);
            g_lk.unlock();
            continue;
        }
        else if (r_lk_state == ERecordLockState::EWAIT)
        {
            std::cout << "tid : " << tid << " record : " << k << " write2 wait\n";
            cv->wait(g_lk);
        }
        std::cout << "tid : " << tid << " record : " << k << " write2 acquire\n";
        g_lk.unlock();
        record_k = db->get_record(k);
        record_k -= record_i;
        db->set_record(k, record_k);

        // committing phase
        g_lk.lock();
        db->rw_unlock(i, tid);
        db->rw_unlock(j, tid);
        db->rw_unlock(k, tid);
        std::cout << " >> " << i << ' ' << j << ' ' << k << ' '
                  << record_i << ' ' << record_j << ' ' << record_k << '\n';
        g_lk.unlock();
        break;
    }
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
