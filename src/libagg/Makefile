
CXX = g++
CXXFLAGS = -Wall -O2 -I../libgambit -I../libagg



default: getpayoffs gampayoffs


getpayoffs: getpayoffs.o agg.o ../libgambit/*.o
	$(CXX) $(CXXFLAGS) -o $@ $^

gampayoffs: getpayoffs.o agg.o ../libgambit/*.o
	$(CXX) $(CXXFLAGS) -o gampayoffs $^

agg.o: agg.h trie_map.h trie_map.template GrayComposition.h proj_func.h

getpayoffs.o: agg.h trie_map.h trie_map.template

clean:
	rm -f *.o getpayoffs
