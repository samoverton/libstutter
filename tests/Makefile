TESTS=helloworld proxy upload

CXXFLAGS=-I..
LDFLAGS=-L.. -levent ../libstutter.a

all: $(TESTS)

# FIXME: how can we factor these three together?

helloworld: helloworld.o
	$(CXX) -o $@ $< $(LDFLAGS)

proxy: proxy.o
	$(CXX) -o $@ $< $(LDFLAGS)

upload: upload.o
	$(CXX) -o $@ $< $(LDFLAGS)
	
%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean: 
	rm -f $(TESTS)
	for t in $(TESTS); do rm -f $$t.o; done
