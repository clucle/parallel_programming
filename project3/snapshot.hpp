#ifndef C_SNAPSHOT
#define C_SNAPSHOT
#include <vector>
#include "register.hpp"
class WaitFreeSnapshot
{
public:
    WaitFreeSnapshot(int N);
    ~WaitFreeSnapshot();
    void update(int tid, int value);
    std::shared_ptr<int> scan();
    int get_cnt();

private:
    SnapValue **collect();
    Register **_register;
    int _size;
};
#endif
