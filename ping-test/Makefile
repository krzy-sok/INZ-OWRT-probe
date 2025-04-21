CXX = gcc

CXXFLAGS = -Wall -Wextra -D_GNU_SOURCE

FILES = ping.c


default: main
# targets
main:
	$(CXX) $(CXXFLAGS) $(FILES) -o my_ping

clean:
	rm -r my_ping
