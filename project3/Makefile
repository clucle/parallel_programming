CFLAGS := -std=c++14

PROG := run
OBJS := main.o
HDRS := #

all: $(PROG)
$(PROG): $(OBJS)
	g++ -o $(PROG) $(OBJS) $(CFLAGS) -lpthread

main.o: main.cpp $(HDRS)
	g++ $(CFLAGS) -c main.cpp -o main.o -I.

clean:
	rm *.o
	rm run
