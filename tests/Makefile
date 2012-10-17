TESTS=helloworld proxy

CXXFLAGS=-I../core
LDFLAGS=-L.. -levent ../libstutter.a

all: $(TESTS)

helloworld: helloworld.o
	$(CXX) -o $@ $< $(LDFLAGS)

proxy: proxy.o
	$(CXX) -o $@ $< $(LDFLAGS)
	
%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean: 
	rm -f $(TESTS)
	for t in $(TESTS); do rm -f $$t.o; done
