#ifndef C_REGISTER
#define C_REGISTER

#define MAX_THREAD 32

#include <cstring>
#include <memory>

class SnapValue
{
public:
    SnapValue();
    SnapValue(int label, int value, std::shared_ptr<int> snap);
    ~SnapValue();

    bool operator==(const SnapValue &other) const;
    bool operator!=(const SnapValue &other) const;

    // private:
    int _label;
    int _value;
    std::shared_ptr<int> _snap;
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
