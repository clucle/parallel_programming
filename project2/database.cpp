#include "database.hpp"

Database::Database(int N, int R, int E) : sz(R)
{
    edges_out = new std::set<int>[N + 1];
    edges_in = new std::set<int>[N + 1];
}

Database::~Database()
{
    delete[] edges_out;
    delete[] edges_in;
}

std::mutex &Database::get_lock()
{
    return m;
}

size_t Database::size()
{
    return sz;
}

ERecordLockState Database::rd_lock(int tid, std::unique_ptr<std::condition_variable> &cv)
{
    // do something
    return ERecordLockState::EWAIT;
}

ERecordLockState Database::wr_lock()
{
}

void Database::rw_unlock()
{
}
