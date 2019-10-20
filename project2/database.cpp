#include "database.hpp"

Database::Database(int N, int R, int E) : sz(R)
{
}

std::mutex &Database::get_lock()
{
    return m;
}

size_t Database::size()
{
    return sz;
}