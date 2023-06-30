CXX = g++
CXXFLAGS = -O3 -std=c++17
LDFLAGS =
INCLUDES = -I /usr/local/include

TARGETS = huffman_seq huffman_thread huffman_ff

all: $(TARGETS)

huffman_seq: huffman_seq.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

huffman_thread: huffman_thread.cpp
	$(CXX) $(CXXFLAGS) -pthread -o $@ $< $(LDFLAGS)

huffman_ff: huffman_ff.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)