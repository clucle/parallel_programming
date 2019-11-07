#ifndef C_REGISTER
#define C_REGISTER

#define MAX_THREAD 32

#include <cstring>

class SnapValue
{
public:
    SnapValue();
    SnapValue(int label, int value, int *snap);
    ~SnapValue();

    bool operator==(const SnapValue &other) const;
    bool operator!=(const SnapValue &other) const;

    // private:
    int _label;
    int _value;
    int *_snap;
};

class Register
{
public:
    Register(int N);
    ~Register();
    SnapValue *read();
    void write(SnapValue *snap_value);

private:
    SnapValue *_snap_value;
};

#endif
