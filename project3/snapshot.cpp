#include "snapshot.hpp"

WaitFreeSnapshot::WaitFreeSnapshot(int N)
{
    _register = new Register *[N];
    for (int i = 0; i < N; i++)
    {
        _register[i] = new Register(N);
    }
    _size = N;
}

WaitFreeSnapshot::~WaitFreeSnapshot()
{
    for (int i = 0; i < _size; i++)
    {
        delete _register[i];
    }
    delete[] _register;
}

void WaitFreeSnapshot::update(int tid, int value)
{
    std::shared_ptr<int> snap = scan();
    SnapValue *old_value = _register[tid]->read();
    old_value->_label++;
    old_value->_snap = snap;
    old_value->_value = value;
    // SnapValue *new_value = new SnapValue(old_value->_label + 1, value, snap);
    // _register[tid]->write(new_value);
}

std::shared_ptr<int> WaitFreeSnapshot::scan()
{
    SnapValue **old_copy;
    SnapValue **new_copy;
    std::vector<bool> moved(_size);
    for (int i = 0; i < _size; i++)
    {
        moved[i] = false;
    }

    old_copy = collect();
    while (true)
    {
        new_copy = collect();
        bool is_clean = true;
        for (int j = 0; j < _size; j++)
        {
            if (old_copy[j] != new_copy[j])
            {
                if (moved[j])
                {
                    std::shared_ptr<int> ret = new_copy[j]->_snap;

                    for (int i = 0; i < _size; i++)
                    {
                        delete old_copy[i];
                        delete new_copy[i];
                    }
                    delete old_copy;
                    delete new_copy;
                    return ret;
                }
                else
                {
                    moved[j] = true;
                    for (int i = 0; i < _size; i++)
                    {
                        delete old_copy[i];
                    }
                    delete old_copy;

                    old_copy = new_copy;
                    is_clean = false;
                    break;
                }
            }
        }
        if (is_clean)
        {
            break;
        }
    }

    std::shared_ptr<int> ret(new int[MAX_THREAD], std::default_delete<int[]>());
    for (int i = 0; i < MAX_THREAD; i++)
    {
        ret.get()[i] = new_copy[i]->_value;
    }

    for (int i = 0; i < _size; i++)
    {
        delete old_copy[i];
        delete new_copy[i];
    }
    delete old_copy;
    delete new_copy;
    return ret;
}

int WaitFreeSnapshot::get_cnt()
{
    int sum = 0;
    for (int i = 0; i < _size; i++)
    {
        SnapValue *temp = _register[i]->read();
        sum += temp->_label;
    }
    return sum;
}

SnapValue **WaitFreeSnapshot::collect()
{
    SnapValue **ret = new SnapValue *[_size];
    for (int i = 0; i < _size; i++)
    {
        SnapValue *temp = _register[i]->read();
        ret[i] = new SnapValue(temp->_label, temp->_value, temp->_snap);
    }
    return ret;
}
