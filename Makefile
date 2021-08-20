APP = serverone
CXX = g++
LIB = -lpthread -lmysqlclient
FLAGS = -std=c++11 -W -Wall -O0 -g

OBJS = main.cpp ./Server/*.cpp ./Log/*.cpp ./Sql/*.cpp

main: $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $(APP) $(LIB)

.PHONY: clean
clean:
	rm -r $(APP)