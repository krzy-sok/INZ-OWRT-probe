CXX = gcc

CXXFLAGS = -Wall -Wextra -D_GNU_SOURCE

FILES = src/arp-reader.c


default: main
# targets
main:
	$(CXX) $(CXXFLAGS) $(FILES) -o arp_read

clean:
	rm -r arp_read
