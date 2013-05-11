CXXFLAGS=$(shell pkg-config --cflags libbitcoin)
LIBS=$(shell pkg-config --libs libbitcoin)
BASE_MODULES= \
    main.o \
    query_app.o
MODULES=$(addprefix obj/, $(BASE_MODULES))

default: queryd

obj/query_app.o: src/query_app.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

obj/main.o: src/main.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

queryd: $(MODULES)
	mkdir -p obj
	$(CXX) -o queryd $(MODULES) $(LIBS)

