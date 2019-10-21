#include "database.hpp"

Database::Database(int N, int R, int E) : sz(R)
{
    edges_out = new std::set<int>[N + 1];
    edges_in = new std::set<int>[N + 1];
    records_state = new ERecordState[R + 1];
    for (int i = 1; i <= R; i++)
    {
        records_state[i] = ERecordState::ESHARE;
    }
}

Database::~Database()
{
    delete[] edges_out;
    delete[] edges_in;
    delete[] records_state;
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
    ERecordLockState ret = ERecordLockState::EWAIT;
    if (records_state[rid] == ERecordState::ESHARE)
    {
        ret = ERecordLockState::EACQUIRE;
    }
    else if (records_state[rid] == ERecordState::EEXECUTE)
    {
        ret = ERecordLockState::EWAIT;
    }
}

ERecordLockState Database::wr_lock()
{
}

void Database::rw_unlock()
{
}
