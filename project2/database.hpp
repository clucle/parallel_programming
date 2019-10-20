#ifndef C_DATABASE
#define C_DATABASE

#include <mutex>

class Database
{
public:
    Database(int N, int R, int E);
    void acquire();
    void release();
    size_t size();

private:
    std::mutex m;
    size_t sz;
};

#endif
