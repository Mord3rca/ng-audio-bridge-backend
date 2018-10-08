CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-std=c++11 -Wall -Ilibnet/include -I/usr/include/jsoncpp
LIBS=-lpthread -lnet -lsqlite3 -ljsoncpp

SRCS= src/Audio-Bridge-Server.cpp src/AudioDatabase.cpp \
			src/filter.cpp src/songItem.cpp src/ng-backend.cpp

OBJS=$(SRCS:.cpp=.o)

all: ng-backend

ng-backend: libnet.a $(OBJS)
	$(CXX) -o ng-backend -Llibnet $(OBJS) $(LIBS)

libnet.a:
	$(MAKE) -C libnet

.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) ng-backend src/*.o

install:
	cp ng-backend /usr/bin/
	cp -Rv www/ /var/www/ng-frontend
