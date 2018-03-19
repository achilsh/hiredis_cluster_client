INC_SVR_COMM = -I..  -I/usr/local/include 
DY_LIBS +=  -lev -levent
LIB_SVR_COMM = -L.. -lhiredis_cluster -L/usr/local/lib ${DY_LIBS}


INC_ALL=$(INC_SVR_COMM)
LIB_ALL=$(LIB_SVR_COMM)

BINARY = cluster_async_libev \
         cluster_sync_test_v1 \
         cluster_sync_test_v2 \
         cluster_async_libevent        


all:$(BINARY)


.SUFFIXES: .o .cc
CXX = g++
CC = gcc

CPPFLAGS= -g -Wall -Bstatic

%.o:%.cc
	$(CXX) $(CPPFLAGS) -c $^ $(INC_ALL) -I.

cluster_async_libev:redis_cluster_async_libev.o
	 $(CXX) $(CPPFLAGS)  -o $@ $^ $(LIB_ALL)

cluster_sync_test_v1:redis_cluster_sync_test_v1.o
	 $(CXX) $(CPPFLAGS)  -o $@ $^ $(LIB_ALL)

cluster_sync_test_v2:redis_cluster_sync_test_v1.o
	 $(CXX) $(CPPFLAGS)  -o $@ $^ $(LIB_ALL)

cluster_async_libevent:redis_cluster_async_libevent.o
	 $(CXX) $(CPPFLAGS)  -o $@ $^ $(LIB_ALL)


clean:
	rm -f *.o *~ $(BINARY)

strip:
	strip $(BINARY)

#install:
#	cp $(BINARY) $(BINPATH)

