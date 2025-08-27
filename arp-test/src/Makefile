CXX = gcc

CXXFLAGS = -Wall -Wextra -D_GNU_SOURCE

FILES = main.cpp classes/host.hpp classes/host.cpp classes/ping_row.hpp classes/ping_row.cpp


default: main
# targets
main:
	$(CXX) $(CXXFLAGS) $(FILES) -o myping -lstdc++ -std=c++17

clean:
	rm -r myping
