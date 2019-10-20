#include "database.hpp"

Database::Database(int N, int R, int E) : sz(R)
{
}

void Database::acquire()
{
    m.lock();
}

void Database::release()
{
    m.unlock();
}

size_t Database::size() {
    return sz;
}