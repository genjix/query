CXXFLAGS=$(shell pkg-config --cflags libbitcoin thrift)
LIBS=$(shell pkg-config --libs libbitcoin thrift) -lzmq
BASE_MODULES= \
    main.o \
    node_impl.o \
    publisher.o \
    sync_blockchain.o \
    interface_types.o \
    query_service.o
MODULES=$(addprefix obj/, $(BASE_MODULES))

default: queryd

obj/interface_types.o: interface.thrift
	mkdir -p src/thrift
	thrift -out src/thrift --gen cpp interface.thrift
	$(CXX) -o $@ -c src/thrift/interface_types.cpp $(CXXFLAGS)

obj/query_service.o: interface.thrift
	mkdir -p src/thrift
	thrift -out src/thrift --gen cpp interface.thrift
	$(CXX) -o $@ -c src/thrift/QueryService.cpp $(CXXFLAGS)

obj/sync_blockchain.o: src/sync_blockchain.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

obj/publisher.o: src/publisher.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

obj/node_impl.o: src/node_impl.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

obj/main.o: src/main.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

queryd: $(MODULES)
	mkdir -p obj
	$(CXX) -o queryd $(MODULES) $(LIBS)

