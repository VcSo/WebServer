APP = serverone
CXX = g++
LIB = -lpthread -lmysqlclient

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	FLAGS = -std=c++11 -Wall -W -g
else
	FLAGS = -std=c++11 -O3
endif

OBJS = main.cpp ./Server/*.cpp ./Log/*.cpp ./Log/Block_queue.hpp ./Sql/*.cpp ./Lock/*.cpp ./Pool/ThreadPool.hpp ./Http/*.cpp ./Timer/*.cpp

main: $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $(APP) $(LIB)

.PHONY: clean
clean:
	rm -r $(APP)