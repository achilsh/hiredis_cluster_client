#ifdef __cplusplus 
    extern "C" {
#endif 

#include "hircluster.h"

#ifdef __cplusplus
    }
#endif

#include <iostream>
#include <string>
#include <map>
#include <sstream>

using namespace std;
namespace Test 
{

class TestHiredisCluster 
{
 public:
    TestHiredisCluster();
    TestHiredisCluster(const std::multimap<std::string, unsigned short>& mpRedisNodes);
    virtual ~TestHiredisCluster();
     
    bool ExcauteCmd(const std::string& strCmd, redisReply *&reply);
    

 private:
    redisClusterContext* m_pClusterCtx;
    bool m_bIsConnnect;
};

TestHiredisCluster::TestHiredisCluster():m_pClusterCtx(NULL), m_bIsConnnect(false)
{
}

TestHiredisCluster::TestHiredisCluster(const std::multimap<std::string,
                                       unsigned short>& mpRedisNodes)
{
    std::multimap<std::string, unsigned short>::const_iterator cnt_it;
    std::stringstream ios;
    for (cnt_it = mpRedisNodes.begin(); cnt_it != mpRedisNodes.end(); ++cnt_it)
    {
        if (cnt_it->first.empty() == false && cnt_it->second >0)
        {
            ios << cnt_it->first << ":" << cnt_it->second << ",";
        }
    }
    std::string strRedisNode = ios.str();
    if (strRedisNode.empty())
    {
        return ;
    }
    strRedisNode = strRedisNode.substr(0, strRedisNode.size()-1);
    std::cout << "redis backend node: " << strRedisNode << "\n";
    
    m_pClusterCtx = redisClusterConnectNonBlock(strRedisNode.c_str(),HIRCLUSTER_FLAG_ROUTE_USE_SLOTS);
    if (m_pClusterCtx != NULL) 
    {
        m_bIsConnnect = true;
        std::cout << "redis cluster client connect succ\n";
    }
}

TestHiredisCluster::~TestHiredisCluster()
{
    if (m_pClusterCtx) 
    {
        redisClusterFree(m_pClusterCtx);
        m_pClusterCtx = NULL;
        m_bIsConnnect = false;
    }
}

bool TestHiredisCluster::ExcauteCmd(const std::string& strCmd, 
                                    redisReply *&reply)
{
    if (strCmd.empty())
    {
        std::cout << "cmd is empty \n";
        return false;
    }

    if (m_bIsConnnect == false)
    {
        std::cout << "not been connect to backend server \n";
        return false;
    }
    
    reply = NULL;
    std::cout << "redis cmd: " << strCmd << std::endl;
    redisReply *replyRet = NULL;
    replyRet = (redisReply *)redisClusterCommand(m_pClusterCtx, strCmd.c_str());
    if (replyRet == NULL)
    {
        std::cout << "ret type: " << m_pClusterCtx->err <<  ","
            << "ret err msg: " <<  m_pClusterCtx->errstr << "\n";

    }
    std::cout << "reply addr: " << replyRet << std::endl; 
    reply = replyRet;
    return true;
}

//
}

using namespace Test;
int main()
{
    std::string sIp = "127.0.0.1";
    std::multimap<std::string, unsigned short> mpRedisNodes;
    mpRedisNodes.insert(std::pair<std::string, unsigned short>(sIp, 7000));
    mpRedisNodes.insert(std::pair<std::string, unsigned short>(sIp, 7001));
    
    TestHiredisCluster testRedisInstance(mpRedisNodes);
   
    std::string strCmdOne = "set 1111 aaaaa";
    redisReply* preply = NULL;
    testRedisInstance.ExcauteCmd(strCmdOne, preply);
    if (preply)
    {
        std::cout << "free first test ret\n";
        std::cout << "ret type: " << preply->type << " ," << preply->str <<  std::endl;
        freeReplyObject(preply);
        preply = NULL;
    }

    std::string strCmdTwo = "set 2222 bbbb";
    testRedisInstance.ExcauteCmd(strCmdTwo, preply);
    if (preply)
    {
        std::cout << "free two test ret\n";
        std::cout << "ret type: " << preply->type << " ," << preply->str << std::endl;
        freeReplyObject(preply);
        preply = NULL;
    }

    
    //read from redis cluster
    std::string strCmdOneRead = "get 1111";
    testRedisInstance.ExcauteCmd(strCmdOneRead, preply);
    if (preply) 
    {
        std::cout << "read 1111 " << "type: " << preply->type << "\n";
        std::cout << "data: " << preply->str << "\n";
        freeReplyObject(preply);
        preply = NULL;
    }

    std::string strCmdTwoRead = "get 2222";
    testRedisInstance.ExcauteCmd(strCmdOneRead, preply);
    if (preply) 
    {
        std::cout << "read 2222 " << "type: " << preply->type << "\n";
        std::cout << "data: " << preply->str << "\n";
        freeReplyObject(preply);
        preply = NULL;
    }



    return 0;
}

