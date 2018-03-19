#include <stdio.h>
#include <sstream>
#include <map>
#include <iostream>

#ifdef __cplusplus 
    extern "C" {
#endif 
    #include "hircluster.h"
    #include "adapters/libev.h"

#ifdef __cplusplus
    }
#endif

namespace Test 
{
    class RedisClusterAsynLibev 
    {
        public:
         RedisClusterAsynLibev();
         RedisClusterAsynLibev(const std::multimap<std::string,
                               unsigned>& mpRedisNodes);
         virtual ~RedisClusterAsynLibev();
         //
         void Run();
         bool RedisConnect(const redisAsyncContext *ac, int status);
         bool RedisDisConnect(const redisAsyncContext *ac, int status);
         //bool RedisCmdProcess();

         static  void RedisCmdCallback(redisClusterAsyncContext *ac, 
                                       void *r, void* pdata);
         static  void RedisConnectCallback(const redisAsyncContext *ac, int status);
         static  void RedisDisConnectCallback(const redisAsyncContext *ac, int status);
        private:
         bool m_isConnect;
         redisClusterAsyncContext *m_pRedisCtx;
         struct ev_loop *m_pLoop;
         int m_iTestNums;
    };
    RedisClusterAsynLibev::RedisClusterAsynLibev()
        :m_isConnect(false), m_pRedisCtx(NULL), m_pLoop(NULL),m_iTestNums(10)
    {
    }

    RedisClusterAsynLibev::~RedisClusterAsynLibev() 
    {
        if (m_pRedisCtx) 
        {
            redisClusterAsyncFree(m_pRedisCtx);
            m_pRedisCtx = NULL;
        }
    }

    RedisClusterAsynLibev::RedisClusterAsynLibev(const std::multimap<std::string, unsigned>& mpRedisNodes)
    {
        std::multimap<std::string, unsigned>::const_iterator it;
        std::stringstream ios;
        for (it = mpRedisNodes.begin(); it != mpRedisNodes.end(); ++it)
        {
            if (it->first.empty() == false && it->second >0)
            {
                ios << it->first << ":" << it->second << ",";
            }
        }
        std::string strRedisNode = ios.str();
        if (strRedisNode.empty())
        {
            return ;
        }
        strRedisNode = strRedisNode.substr(0, strRedisNode.size()-1);
        std::cout << "redis node: " << strRedisNode << "\n";

        m_pRedisCtx = redisClusterAsyncConnect(strRedisNode.c_str(), 
                                               HIRCLUSTER_FLAG_ROUTE_USE_SLOTS);
        if (m_pRedisCtx->err)
        {
            std::cout << "async connect faild, err Msg: "
                << m_pRedisCtx->errstr << "\n";
            return ;
        }
        
        m_pLoop =  ev_loop_new(EVFLAG_AUTO);
        redisClusterLibevAttach(m_pLoop, m_pRedisCtx);
        redisClusterAsyncSetConnectCallback(m_pRedisCtx, RedisConnectCallback);
        redisClusterAsyncSetDisconnectCallback(m_pRedisCtx, RedisDisConnectCallback);
        m_isConnect = true;
        m_pRedisCtx->data = this;
        m_iTestNums = 10;
    }
    void RedisClusterAsynLibev::RedisDisConnectCallback(const redisAsyncContext *ac, int status) 
    {
        if (ac->data != NULL)
        {
            RedisClusterAsynLibev* pMain = (RedisClusterAsynLibev*)ac->data;
            pMain->RedisDisConnect(ac, status);
        }
    }
    bool RedisClusterAsynLibev::RedisDisConnect(const redisAsyncContext *ac, int status)
    {
        if (status  != REDIS_OK)
        {
            std::cout << "disconnect error : " << ac->errstr << "\n";
            return false;
        }
        std::cout << "disconnected ok \n";
        return true;
    }

    void RedisClusterAsynLibev::RedisConnectCallback(const redisAsyncContext *ac, int status)
    {
        if (ac->data != NULL)
        {
            RedisClusterAsynLibev* pMain = (RedisClusterAsynLibev*)ac->data;
            pMain->RedisConnect(ac, status);
        }
    }
    bool RedisClusterAsynLibev::RedisConnect(const redisAsyncContext *ac, int ret_status)
    {
        if (ret_status  != REDIS_OK)
        {
            std::cout << "connect error : " << ac->errstr << "\n";
            return false;
        }

        std::cout << "connected ok, next step is send cmd \n";
        /***    
        int status; 
        for (int i = 0; i < m_iTestNums; ++i) 
        {
            std::stringstream  ios;
            ios << "libev:" << i;
            std::cout << "send cmd: " << "set " << ios.str() << " " << i << "\n";

            status = redisClusterAsyncCommand(
                m_pRedisCtx, RedisCmdCallback, &m_iTestNums,
                "set %s %d", ios.str().c_str(), i);
            if (status != REDIS_OK)
            {
                std::cout << "error no: " << m_pRedisCtx->err
                    << ", err msg: " << m_pRedisCtx->errstr << "\n";
            }
        }
        ***/
        return true;
    }
    void RedisClusterAsynLibev::Run()
    {
        if (m_isConnect  == false)
        {
            std::cout << "redis not connect \n";
            return ;
        }

        int status; 
        for (int i = 0; i < m_iTestNums; ++i) 
        {
            std::stringstream  ios;
            ios << "libev:" << i;
            std::cout << "send cmd: " << "set " << ios.str() << " " << i << "\n";

            status = redisClusterAsyncCommand(
                m_pRedisCtx, RedisCmdCallback, &m_iTestNums,
                "set %s %d", ios.str().c_str(), i);
            if (status != REDIS_OK)
            {
                std::cout << "error no: " << m_pRedisCtx->err
                    << ", err msg: " << m_pRedisCtx->errstr << "\n";
            }
        }
        ev_loop(m_pLoop, 0);
    }

    void RedisClusterAsynLibev::RedisCmdCallback(redisClusterAsyncContext *ac, 
                                                 void *r, 
                                                 void* pdata)
    {
        //redisReply *reply = (redisReply *)r;
        int count = *(int*)pdata;
        static int all_count = 0;
        all_count ++;
        if (all_count >= count)
        {
            //std::cout << "close connect\n";
            //redisClusterAsyncDisconnect(ac);
        }
        std::cout << "redis cmd call, num: " << all_count << "\n";
    }
}


using namespace Test;
int main()
{
    std::multimap<std::string, unsigned> redisNodes;
    redisNodes.insert(std::pair<std::string, unsigned>(
            "127.0.0.1", 7000));

    RedisClusterAsynLibev test(redisNodes);
    test.Run();
    return 0;
}
