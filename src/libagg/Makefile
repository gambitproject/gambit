
CXX = g++
CXXFLAGS = -Wall -O2



default: getpayoffs


getpayoffs: getpayoffs.o agg.o
	$(CXX) $(CXXFLAGS) -o $@ $^

agg.o: agg.h trie_map.h trie_map.template GrayComposition.h proj_func.h

getpayoffs.o: agg.h trie_map.h trie_map.template

clean:
	rm -f *.o getpayoffs
