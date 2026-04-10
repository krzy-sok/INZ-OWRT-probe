CXX = gcc

CXXFLAGS = -Wall -Wextra -D_GNU_SOURCE

FILES = $(shell find $(SOURCE_DIR) -name '*.hpp' -o -name '*.cpp')


default: main
# targets
main:
	$(CXX) $(CXXFLAGS) $(FILES) -o myping -lcurl -lstdc++ -std=c++2a -lm

clean:
	rm -r myping
