CXX = gcc

CXXFLAGS = -Wall -Wextra -D_GNU_SOURCE

FILES = ping.c


default: main
# targets
main:
	$(CXX) $(CXXFLAGS) $(FILES) -o my_ping


sys_ping:
	$(CXX) $(CXXFLAGS) system_ping.c -o sys_ping

clean:
	rm -r my_ping, sys_ping
