#ifndef C_DATABASE
#define C_DATABASE

#include <mutex>

class Database {
public:
    Database(int N, int R, int E);
    void acquire();
    void release();
private:
    std::mutex m;
};

#endif
