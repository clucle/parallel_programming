#ifndef C_DATABASE
#define C_DATABASE

#include <mutex>

class Database
{
public:
    Database(int N, int R, int E);
    size_t size();
    std::mutex &get_lock();

private:
    std::mutex m;
    size_t sz;
};

#endif
