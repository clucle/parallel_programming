#ifndef C_DATABASE
#define C_DATABASE

#include <mutex>
#include <set>
#include <condition_variable>
#include <list>
#include <queue>
#include "recordlock.hpp"

class Database
{
public:
    Database(int N, int R, int E);
    ~Database();
    size_t size();
    std::mutex &get_lock();
    ERecordLockState rd_lock(int rid, int tid, std::unique_ptr<std::condition_variable> &cv);
    ERecordLockState wr_lock(int rid, int tid, std::unique_ptr<std::condition_variable> &cv);
    void rw_unlock(int rid, int tid);

private:
    std::mutex m;
    size_t sz;
    std::set<int> *edges_out;
    std::set<int> *edges_in;
    ERecordState *arr_record_state;
    std::list<RecordLock *> *arr_list_record_lock;

    bool is_deadlock(int rid, int tid);
    void remove_edges_dependency(int tid);
};

#endif
