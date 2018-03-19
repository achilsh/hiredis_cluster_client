#include <stdio.h>

#ifdef __cplusplus 
    extern "C" {
#endif 

#include "hircluster.h"
#include "adapters/libevent.h"
#include <event2/event.h>
#include "async.h"
 
#ifdef __cplusplus
    }
#endif

int all_count=0;

typedef struct calldata
{
    redisClusterAsyncContext *acc;
    int count;
}calldata;

void getCallback(redisClusterAsyncContext *acc, void *r, void *privdata)
{
    //redisReply *reply = (redisReply *)r;
    int count =  *(int*)privdata;
    all_count ++;
    if(all_count >= count)
    {
        redisClusterAsyncDisconnect(acc);
    }
}

void connectCallback(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    
    printf("\nDisconnected...\n");
}

int main(int argc, char **argv)
{
    int status, i;
    struct event_base *base = event_base_new();
    redisClusterAsyncContext *acc = redisClusterAsyncConnect("127.0.0.1:7000", 
                                                             HIRCLUSTER_FLAG_ROUTE_USE_SLOTS
                                                             /*    HIRCLUSTER_FLAG_NULL*/);
    if (acc->err)
    {
        printf("Error: %s\n", acc->errstr);
        return 1;
    }
    redisClusterLibeventAttach(acc,base);
    redisClusterAsyncSetConnectCallback(acc,connectCallback);
    redisClusterAsyncSetDisconnectCallback(acc,disconnectCallback);
    
    int count = 100;
    for(i = 0; i < count; i ++)
    {
        status = redisClusterAsyncCommand(acc, getCallback, &count, "set %d %d", i, i);
        if(status != REDIS_OK)
        {
            printf("error: %d %s\n", acc->err, acc->errstr);
        }
    }
    
    event_base_dispatch(base);
    return 0;
}
