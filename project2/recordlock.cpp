#include "recordlock.hpp"

RecordLock::RecordLock(int tid, ERecordState r_state, std::unique_ptr<std::condition_variable> &cv)
    : tid(tid), r_state(r_state), cv(cv)
{
}

ERecordLockState RecordLock::get_record_lock_state()
{
    return r_lk_state;
}

void RecordLock::set_record_lock_state(ERecordLockState state)
{
    r_lk_state = state;
}
