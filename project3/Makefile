CFLAGS := -std=c++14

PROG := run
OBJS := main.o register.o
HDRS := register.hpp

all: $(PROG)
$(PROG): $(OBJS)
	g++ -o $(PROG) $(OBJS) $(CFLAGS) -lpthread

main.o: main.cpp $(HDRS)
	g++ $(CFLAGS) -c main.cpp -o main.o -I.

register.o: register.cpp $(HDRS)
	g++ $(CFLAGS) -c register.cpp -o register.o -I.

clean:
	rm *.o
	rm run
