LIB=libstutter.so
ARCHIVE=libstutter.a

HTTP_PARSER_OBJS=stutter/http/joyent/http_parser.o
HANDLER_OBJS=stutter/handlers/base.o stutter/handlers/hello.o stutter/handlers/file.o stutter/handlers/error.o
HTTP_OBJS=stutter/http/connection.o stutter/http/request.o stutter/http/reply.o stutter/http/parser.o stutter/http/message.o stutter/http/body.o stutter/http/proxy.o
CORE_OBJS=stutter/coroutine.o stutter/server.o stutter/pool.o stutter/log.o stutter/dispatcher.o
OBJS=$(CORE_OBJS) $(HANDLER_OBJS) $(HTTP_PARSER_OBJS) $(HTTP_OBJS)
CXXFLAGS=-O0 -ggdb -Wall -Wextra -I. -fPIC
LDFLAGS=-shared

all: $(LIB) $(ARCHIVE) tests

$(LIB): $(OBJS)
	$(CXX) -o $(LIB) $(OBJS) $(LDFLAGS)

$(ARCHIVE): $(OBJS)
	ar rcs $(ARCHIVE) $(OBJS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.c
	$(CXX) $(CXXFLAGS) -o $@ -c $<

tests: $(LIB) $(ARCHIVE)
	make -C tests/ all

clean:
	rm -f $(LIB) $(ARCHIVE) $(OBJS)
	make -C tests/ clean
