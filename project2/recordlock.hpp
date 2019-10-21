#ifndef CRECORD_LOCK
#define CRECORD_LOCK

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
};

#endif
