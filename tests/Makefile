TESTS=helloworld proxy upload blocking

CXXFLAGS=
LDFLAGS=-levent -lstutter -pthread

all: $(TESTS)

# FIXME: how can we factor all these together?

helloworld: helloworld.o
	$(CXX) -o $@ $< $(LDFLAGS)

proxy: proxy.o
	$(CXX) -o $@ $< $(LDFLAGS)

upload: upload.o
	$(CXX) -o $@ $< $(LDFLAGS)

blocking: blocking.o
	$(CXX) -o $@ $< $(LDFLAGS)
	
%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean: 
	rm -f $(TESTS)
	for t in $(TESTS); do rm -f $$t.o; done
