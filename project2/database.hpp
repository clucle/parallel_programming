#ifndef C_DATABASE
#define C_DATABASE

#include <mutex>
#include <set>

class Database
{
public:
    Database(int N, int R, int E);
    ~Database();
    size_t size();
    std::mutex &get_lock();

private:
    std::mutex m;
    size_t sz;
    std::set<int> *edges_out;
    std::set<int> *edges_in;
};

#endif
