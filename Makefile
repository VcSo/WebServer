APP = serverone
CXX = g++
LIB = -lpthread -lmysqlclient
FLAGS = -std=c++11  -g

OBJS = main.cpp ./Server/*.cpp ./Log/*.cpp ./Log/Block_queue.hpp ./Sql/*.cpp ./Lock/*.cpp

main: $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $(APP) $(LIB)

.PHONY: clean
clean:
	rm -r $(APP)