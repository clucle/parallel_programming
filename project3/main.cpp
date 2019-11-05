#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage : ./run N\n";
        return 0;
    }

    int N = atoi(argv[1]);

    // TODO:
    // Make worker thread and update snapshot
}