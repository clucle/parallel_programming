#include "database.hpp"

Database::Database(int N, int R, int E) : sz(R)
{
    edges_out = new std::set<int>[N + 1];
    edges_in = new std::set<int>[N + 1];
    arr_record_state = new ERecordState[R + 1];
    arr_list_record_lock = new std::list<RecordLock *>[R + 1];
    for (int i = 1; i <= R; i++)
    {
        arr_record_state[i] = ERecordState::ESHARE;
    }
}

Database::~Database()
{
    delete[] edges_out;
    delete[] edges_in;
    delete[] arr_record_state;
    delete[] arr_list_record_lock;
}

std::mutex &Database::get_lock()
{
    return m;
}

size_t Database::size()
{
    return sz;
}

ERecordLockState Database::rd_lock(int rid, int tid, std::unique_ptr<std::condition_variable> &cv)
{
    RecordLock *r_lk = new RecordLock(tid, ERecordState::ESHARE, cv);

    if (arr_record_state[rid] == ERecordState::ESHARE)
    {
        r_lk->set_record_lock_state(ERecordLockState::EACQUIRE);
    }
    else if (arr_record_state[rid] == ERecordState::EEXECUTE)
    {
        bool flag_dependency = false;
        for (auto iter = arr_list_record_lock[rid].rbegin();
             iter != arr_list_record_lock[rid].rend();
             --iter)
        {
            RecordLock *r_lk_iter = *iter;
            if (!flag_dependency && r_lk_iter->get_record_state() == ERecordState::ESHARE)
            {
                continue;
            }
            flag_dependency = true;
            edges_in[tid].insert(r_lk_iter->get_tid());
            edges_out[r_lk_iter->get_tid()].insert(tid);
            std::cout << r_lk_iter->get_tid() << "_ insert " << tid << '\n';
        }
        r_lk->set_record_lock_state(ERecordLockState::EWAIT);
    }
    ERecordLockState ret = r_lk->get_record_lock_state();
    arr_list_record_lock[rid].push_back(std::move(r_lk));
    return ret;
}

ERecordLockState Database::wr_lock(int rid, int tid, std::unique_ptr<std::condition_variable> &cv)
{
    bool flag_deadlock = is_deadlock(rid, tid);
    if (flag_deadlock)
    {
        remove_edges_dependency(tid);
        return ERecordLockState::EDEADLOCK;
    }
    arr_record_state[rid] = ERecordState::EEXECUTE;
    RecordLock *r_lk = new RecordLock(tid, ERecordState::EEXECUTE, cv);
    if (arr_list_record_lock[rid].size() == 0)
    {
        r_lk->set_record_lock_state(ERecordLockState::EACQUIRE);
    }
    else
    {
        for (auto iter = arr_list_record_lock[rid].rbegin();
             iter != arr_list_record_lock[rid].rend();
             --iter)
        {
            RecordLock *r_lk_iter = *iter;
            edges_in[tid].insert(r_lk_iter->get_tid());
            edges_out[r_lk_iter->get_tid()].insert(tid);
            std::cout << r_lk_iter->get_tid() << "_ insert " << tid << '\n';
        }
        r_lk->set_record_lock_state(ERecordLockState::EWAIT);
    }
    ERecordLockState ret = r_lk->get_record_lock_state();
    arr_list_record_lock[rid].push_back(std::move(r_lk));
    return ret;
}

void Database::rw_unlock(int rid, int tid)
{
    remove_edges_dependency(tid);
    for (auto iter = arr_list_record_lock[rid].begin();
         iter != arr_list_record_lock[rid].end();
         ++iter)
    {
        RecordLock *r_lk_iter = *iter;
        if (r_lk_iter->get_tid() == tid)
        {
            arr_list_record_lock[rid].erase(iter);
            break;
        }
    }
    RecordLock *r_lk_first = *arr_list_record_lock[rid].begin();
    if (r_lk_first->get_record_lock_state() == ERecordLockState::EWAIT)
    {
        if (r_lk_first->get_record_state() == ERecordState::ESHARE)
        {
            arr_record_state[rid] = ERecordState::ESHARE;
            for (auto iter = arr_list_record_lock[rid].begin();
                 iter != arr_list_record_lock[rid].end();
                 ++iter)
            {
                RecordLock *r_lk_iter = *iter;
                if (r_lk_iter->get_record_state() == ERecordState::EEXECUTE)
                {
                    arr_record_state[rid] = ERecordState::EEXECUTE;
                    break;
                }
                r_lk_iter->wake_up_worker();
                r_lk_iter->set_record_lock_state(ERecordLockState::EACQUIRE);
            }
        }
        else
        {
            arr_record_state[rid] = ERecordState::EEXECUTE;
            r_lk_first->wake_up_worker();
            r_lk_first->set_record_lock_state(ERecordLockState::EACQUIRE);
        }
    }
}

bool Database::is_deadlock(int rid, int tid)
{
    std::cout << "dead lock check rid : " << rid << ' ' << "tid : " << tid << '\n';
    std::set<int> tid_should_not_go;
    for (auto iter = arr_list_record_lock[rid].begin();
         iter != arr_list_record_lock[rid].end();
         ++iter)
    {
        RecordLock *r_lk = *iter;
        std::cout << r_lk->get_tid() << ' ';
        tid_should_not_go.insert(r_lk->get_tid());
    }
    std::set<int> visited;
    std::queue<int> q;
    std::cout << "should not go\n";
    for (auto e : tid_should_not_go)
    {
        std::cout << e << ' ';
    }
    std::cout << '\n';

    q.push(tid);
    visited.insert(tid);
    while (!q.empty())
    {
        int here = q.front();
        q.pop();
        if (tid_should_not_go.find(here) != tid_should_not_go.end())
        {
            return true;
        }
        for (auto e : edges_out[here])
        {
            if (visited.find(e) != visited.end())
                continue;
            visited.insert(e);
            q.push(e);
        }
    }
    return false;
}

void Database::remove_edges_dependency(int tid)
{
    for (auto e : edges_in[tid])
    {
        edges_out[e].erase(tid);
    }
    edges_in[tid].clear();
}