TESTS=helloworld

all: $(TESTS)

helloworld:
	g++ helloworld/helloworld.cpp -o helloworld/helloworld -I../core -lstutter -L.. -levent

clean: $(TESTS)
	for t in $(TESTS); do \
		rm -f $$t/$$t; \
	done
