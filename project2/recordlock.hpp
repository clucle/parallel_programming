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
    RecordLock(int tid, ERecordState state, std::unique_ptr<std::condition_variable> &cv);

private:
    int tid;
    ERecordState state;
    std::unique_ptr<std::condition_variable> &cv;
};

#endif
