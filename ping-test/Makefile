CXX = gcc

CXXFLAGS = -Wall -Wextra -D_GNU_SOURCE

FILES = ping.c


default: main
# targets
main:
	$(CXX) $(CXXFLAGS) $(FILES) -o my_ping

cc:
	$(CC) $(CFLAGS) -o ./myping.o -c ./ping.c
	$(CC) $(LDFLAGS) -o ./my_ping ./myping.o

clean:
	rm -r my_ping
