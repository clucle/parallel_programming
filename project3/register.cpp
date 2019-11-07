#include "register.hpp"

SnapValue::SnapValue()
{
    _label = 0;
    _value = 0;
    _snap = new int[MAX_THREAD];
    for (int i = 0; i < MAX_THREAD; i++)
    {
        _snap[i] = 0;
    }
}

SnapValue::SnapValue(int label, int value, int *snap)
    : _label(label), _value(value)
{
    _snap = new int[MAX_THREAD];
    memcpy(_snap, snap, sizeof(int) * MAX_THREAD);
}

SnapValue::~SnapValue()
{
    delete[] _snap;
}

bool SnapValue::operator==(const SnapValue &other) const
{
    return _label == other._label;
}

bool SnapValue::operator!=(const SnapValue &other) const
{
    return _label != other._label;
}

// Register

Register::Register(int N)
{
    SnapValue *snap_value = new SnapValue();
    _snap_value = snap_value;
}

Register::~Register()
{
    delete _snap_value;
}

SnapValue *Register::read()
{
    return _snap_value;
}

void Register::write(SnapValue *snap_value)
{
    _snap_value->_snap = snap_value->_snap;
    _snap_value->_label = snap_value->_label;
    _snap_value->_value = snap_value->_value;
}
