#ifndef C_DATABASE
#define C_DATABASE

#include <mutex>
#include <set>
#include <condition_variable>
#include "recordlock.hpp"

class Database
{
public:
    Database(int N, int R, int E);
    ~Database();
    size_t size();
    std::mutex &get_lock();
    ERecordLockState rd_lock(int rid, int tid, std::unique_ptr<std::condition_variable> &cv);
    ERecordLockState wr_lock();
    void rw_unlock();

private:
    std::mutex m;
    size_t sz;
    std::set<int> *edges_out;
    std::set<int> *edges_in;
    ERecordState *records_state;
};

#endif
