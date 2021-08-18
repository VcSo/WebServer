APP = WebServer
CXX = g++
LIB = -lpthread -lmysqlclient
FLAGS = -std=c++14 -Wall -g

OBJS = main.cpp ./Server/*.cpp ./ThreadPool/ThreadPool.hpp ./Timer/*.cpp ./Http/*.cpp

all: $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $(APP) $(LIB)

.PHONY: clean
clean:
	rm -r $(APP)