#include "recordlock.hpp"

RecordLock::RecordLock(int tid, ERecordState r_state, std::unique_ptr<std::condition_variable> &cv)
    : tid(tid), r_state(r_state), cv(cv)
{
}

int RecordLock::get_tid()
{
    return tid;
}

ERecordState RecordLock::get_record_state() {
    return r_state;
}

ERecordLockState RecordLock::get_record_lock_state()
{
    return r_lk_state;
}

void RecordLock::set_record_lock_state(ERecordLockState state)
{
    r_lk_state = state;
}
