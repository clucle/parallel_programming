#include "database.hpp"

Database::Database(int N, int R, int E) : sz(R)
{
    edges_out = new std::set<int>[N + 1];
    edges_in = new std::set<int>[N + 1];
    arr_record_state = new ERecordState[R + 1];
    arr_list_record_lock = new std::list<RecordLock *>[R + 1];
    for (int i = 1; i <= R; i++)
    {
        arr_record_state[i] = ERecordState::ESHARE;
    }
}

Database::~Database()
{
    delete[] edges_out;
    delete[] edges_in;
    delete[] arr_record_state;
    delete[] arr_list_record_lock;
}

std::mutex &Database::get_lock()
{
    return m;
}

size_t Database::size()
{
    return sz;
}

ERecordLockState Database::rd_lock(int rid, int tid, std::unique_ptr<std::condition_variable> &cv)
{
    RecordLock *r_lk = new RecordLock(tid, ERecordState::ESHARE, cv);

    if (arr_record_state[rid] == ERecordState::ESHARE)
    {
        r_lk->set_record_lock_state(ERecordLockState::EACQUIRE);
    }
    else if (arr_record_state[rid] == ERecordState::EEXECUTE)
    {
        bool flag_dependency = false;
        for (auto iter = arr_list_record_lock[rid].rbegin();
             iter != arr_list_record_lock[rid].rend();
             --iter)
        {
            RecordLock *r_lk = *iter;
            if (!flag_dependency && r_lk->get_record_state() == ERecordState::ESHARE)
            {
                continue;
            }
            flag_dependency = true;
            edges_in[tid].insert(r_lk->get_tid());
            edges_out[r_lk->get_tid()].insert(tid);
        }
        r_lk->set_record_lock_state(ERecordLockState::EWAIT);
    }

    arr_list_record_lock[rid].push_back(std::move(r_lk));
    return r_lk->get_record_lock_state();
}

ERecordLockState Database::wr_lock()
{
}

void Database::rw_unlock()
{
}
