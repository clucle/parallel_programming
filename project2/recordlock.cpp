#include "recordlock.hpp"

RecordLock::RecordLock(int tid, ERecordState state, std::unique_ptr<std::condition_variable> &cv)
    : tid(tid), state(state), cv(cv)
{
}