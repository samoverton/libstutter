LIB=libstutter.so
ARCHIVE=libstutter.a

# objects
HTTP_PARSER_OBJS=stutter/http/joyent/http_parser.o
HANDLER_OBJS=stutter/handlers/base.o stutter/handlers/hello.o stutter/handlers/file.o stutter/handlers/error.o
HTTP_OBJS=stutter/http/connection.o stutter/http/request.o stutter/http/reply.o stutter/http/parser.o stutter/http/message.o stutter/http/body.o stutter/http/proxy.o
CORE_OBJS=stutter/coroutine.o stutter/server.o stutter/pool.o stutter/log.o stutter/dispatcher.o
OBJS=$(CORE_OBJS) $(HANDLER_OBJS) $(HTTP_PARSER_OBJS) $(HTTP_OBJS)
CXXFLAGS=-O0 -ggdb -Wall -Wextra -I. -fPIC
LDFLAGS=-shared

# install
HEADERS_MAIN=stutter/server.h stutter/coroutine.h stutter/pool.h \
			 stutter/log.h stutter/dispatcher.h
HEADERS_HTTP=stutter/http/connection.h stutter/http/parser.h \
			 stutter/http/request.h stutter/http/reply.h stutter/http/message.h \
			 stutter/http/body.h stutter/http/proxy.h

all: $(LIB) $(ARCHIVE) tests

PREFIX ?= /usr
INCLUDE_DIR=$(PREFIX)/include

INSTALL_DIRS = $(DESTDIR) \
			   $(INCLUDE_DIR)/stutter \
			   $(INCLUDE_DIR)/stutter/http \
			   $(INCLUDE_DIR)/stutter/handlers \
			   $(DESTDIR)/$(PREFIX)/lib

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

install: $(LIB) $(ARCHIVE) $(INSTALL_DIRS)
	cp $(LIB) $(ARCHIVE) $(DESTDIR)/$(PREFIX)/lib
	cp $(HEADERS_MAIN) $(INCLUDE_DIR)/stutter
	cp stutter/handlers/base.h $(INCLUDE_DIR)/stutter/handlers
	cp $(HEADERS_HTTP) $(INCLUDE_DIR)/stutter/http

$(INSTALL_DIRS):
	mkdir -p $@

tests: $(LIB) $(ARCHIVE)
	make -C tests/ all

clean:
	rm -f $(LIB) $(ARCHIVE) $(OBJS)
	make -C tests/ clean
