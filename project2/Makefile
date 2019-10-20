CFLAGS := -std=c++14

PROG := run
OBJS := main.o database.o worker.o
HDRS := database.hpp worker.hpp

all: $(PROG)
$(PROG): $(OBJS)
	g++ -o $(PROG) $(OBJS) $(CFLAGS)

main.o: main.cpp $(HDRS)
	g++ $(CFLAGS) -c main.cpp -o main.o -I.

worker.o: worker.cpp $(HDRS)
	g++ $(CFLAGS) -c worker.cpp -o worker.o -I.

database.o: database.cpp $(HDRS)
	g++ $(CFLAGS) -c database.cpp -o database.o -I.

clean:
	rm *.o
	rm run