OUT=test

HTTP_PARSER_OBJS=core/http-parser/http_parser.o
CORE_OBJS=core/main.o core/coroutine.o core/server.o core/http_client.o core/http_request.o core/http_reply.o
OBJS=$(CORE_OBJS) $(HTTP_PARSER_OBJS)
CXXFLAGS=-std=c++0x -O3 -Wall -Wextra
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
