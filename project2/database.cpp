#include "database.hpp"

Database::Database(int N, int R, int E) {

}

void Database::acquire() {
    m.lock();
}

void Database::release() {
    m.unlock();
}
