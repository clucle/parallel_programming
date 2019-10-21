#ifndef CRECORD_LOCK
#define CRECORD_LOCK

#include <condition_variable>

enum class ERecordLockState
{
    EWAIT,
    EACQUIRE,
    EDEADLOCK
};

enum class ERecordState
{
    ESHARE,
    EEXECUTE
};

class RecordLock
{
public:
    RecordLock(int tid, ERecordState r_state, std::unique_ptr<std::condition_variable> &cv);
    int get_tid();
    ERecordState get_record_state();
    ERecordLockState get_record_lock_state();
    void set_record_lock_state(ERecordLockState state);
    void wake_up_worker();

private:
    int tid;
    ERecordState r_state;
    ERecordLockState r_lk_state;
    std::unique_ptr<std::condition_variable> &cv;
};

#endif
