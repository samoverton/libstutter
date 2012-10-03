OUT=test

HTTP_PARSER_OBJS=http-parser/http_parser.o
OBJS=main.o coroutine.o server.o http_client.o $(HTTP_PARSER_OBJS)
CXXFLAGS=-std=c++0x -ggdb -O0
LDFLAGS=-levent

all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) -o $(OUT) $(OBJS) $(LDFLAGS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.c
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OUT) $(OBJS)
