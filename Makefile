CXXFLAGS=$(shell pkg-config --cflags libbitcoin)
LIBS=$(shell pkg-config --libs libbitcoin)
DEPS=-lzmq
BASE_MODULES= \
    main.o \
    query_app.o \
    publisher.o \
    sync_blockchain.o
MODULES=$(addprefix obj/, $(BASE_MODULES))

default: queryd

obj/sync_blockchain.o: src/sync_blockchain.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

obj/publisher.o: src/publisher.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

obj/query_app.o: src/query_app.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

obj/main.o: src/main.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

queryd: $(MODULES)
	mkdir -p obj
	$(CXX) -o queryd $(MODULES) $(LIBS) $(DEPS)

