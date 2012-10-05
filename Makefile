OUT=test

HTTP_PARSER_OBJS=core/http/joyent/http_parser.o
HANDLER_OBJS=core/handlers/base.o core/handlers/hello.o core/handlers/simple_proxy.o
CORE_OBJS=core/main.o core/coroutine.o core/server.o core/http/connection.o core/http/request.o core/http/reply.o core/http/parser.o
OBJS=$(CORE_OBJS) $(HANDLER_OBJS) $(HTTP_PARSER_OBJS)
CXXFLAGS=-std=c++0x -O0 -ggdb -Wall -Wextra -I.
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
