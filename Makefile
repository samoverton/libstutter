OUT=test

OBJS=main.o coroutine.o server.o
CXXFLAGS=-std=c++0x -ggdb -O0

all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) -o $(OUT) $(OBJS) $(LDFLAGS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OUT) $(OBJS)
