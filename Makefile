LIB=libstutter.so
ARCHIVE=libstutter.a

HTTP_PARSER_OBJS=core/http/joyent/http_parser.o
HANDLER_OBJS=core/handlers/base.o core/handlers/hello.o core/handlers/simple_proxy.o core/handlers/file.o
HTTP_OBJS=core/http/connection.o core/http/request.o core/http/reply.o core/http/parser.o core/http/message.o core/http/body.o core/http/proxy.o
CORE_OBJS=core/coroutine.o core/server.o core/pool.o core/log.o
OBJS=$(CORE_OBJS) $(HANDLER_OBJS) $(HTTP_PARSER_OBJS) $(HTTP_OBJS)
CXXFLAGS=-O0 -ggdb -Wall -Wextra -I. -fPIC
LDFLAGS=-shared

all: $(LIB) $(ARCHIVE)

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

clean:
	rm -f $(LIB) $(ARCHIVE) $(OBJS)
