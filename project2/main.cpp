#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        std::cerr << "Usage : ./run N R E\n";
        return 0;
    }

    int N = atoi(argv[1]);
    int R = atoi(argv[2]);
    int E = atoi(argv[3]);

    if (R < 3)
    {
        std::cerr << "number of records is greater or equal than 3\n";
        return 0;
    }
}
