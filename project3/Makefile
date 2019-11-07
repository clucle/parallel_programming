CFLAGS := -std=c++14

PROG := run
OBJS := main.o worker.o snapshot.o register.o
HDRS := worker.hpp snapshot.hpp register.hpp

all: $(PROG)
$(PROG): $(OBJS)
	g++ -o $(PROG) $(OBJS) $(CFLAGS) -lpthread

main.o: main.cpp $(HDRS)
	g++ $(CFLAGS) -c main.cpp -o main.o -I.

register.o: register.cpp $(HDRS)
	g++ $(CFLAGS) -c register.cpp -o register.o -I.

snapshot.o: snapshot.cpp $(HDRS)
	g++ $(CFLAGS) -c snapshot.cpp -o snapshot.o -I.

worker.o: worker.cpp $(HDRS)
	g++ $(CFLAGS) -c worker.cpp -o worker.o -I.

clean:
	rm *.o
	rm run
