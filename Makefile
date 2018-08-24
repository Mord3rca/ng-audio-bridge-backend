CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-std=c++11 -Wall -I/usr/include/jsoncpp
LIBS=-lpthread -lsqlite3 -ljsoncpp

SRCS= src/Audio-Bridge-Server.cpp src/AudioDatabase.cpp \
			src/filter.cpp src/HTTPServer.cpp src/songItem.cpp src/TCPEventLoop.cpp \
			src/ng-backend.cpp

OBJS=$(SRCS:.cpp=.o)

all: ng-backend

ng-backend: $(OBJS)
	$(CXX) -o ng-backend $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) ng-backend src/*.o

install:
	cp ng-backend /usr/bin/
	cp -Rv www/ /var/www/ng-frontend
