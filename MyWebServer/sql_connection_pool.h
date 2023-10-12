/*
// 数据库连接池
// 在处理用户注册，登录请求的时候，我们需要将这些用户的用户名和密码保存下来用于新用户的注册及
// 老用户的登录校验。若每次用户请求我们都需要新建一个数据库连接，请求结束后我们释放该数据库连
// 接，当用户请求连接过多时，这种做法过于低效，所以类似线程池的做法，所以构建一个数据库连接
// 池，预先生成一些数据库连接放在那里供用户请求使用。
*/
#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <string.h>
#include <iostream>
#include <mysql/mysql.h>
#include <list>
#include "locker.h"

class connection_pool
{
public:
    MYSQL* GetConnection(); // 获取数据库连接
    bool ReleaseConnection(MYSQL* conn); // 释放连接
    int GetFreeConn(); // 获取连接
    void DestoryPool(); // 销毁所有连接

    static connection_pool* GetInstance();

    void init(std::string url, std::string user, std::string password, std::string DataBaseName, 
                int Port, int MaxConn, int close_log);

private:
    connection_pool();
    ~connection_pool();

    int m_MaxConn; // 最大连接数
    int m_CurConn; // 当前已使用的连接数
    int m_FreeConn; // 当前空闲的连接数
    locker lock;
    std::list<MYSQL*> m_connList; // 连接池
    sem m_reserve;

public:
    std::string m_url; // 主机地址
    std::string m_port; // 数据库端口号
    std::string m_user; // 登陆数据库用户名
    std::string m_passWord; // 登陆数据库密码
    std::string m_databaseName; // 数据库名
    int m_close_log; // 日志开关
};

class connectionRALL 
{
public:
    connectionRALL(MYSQL** con, connection_pool* connPool);
    ~connectionRALL();

private:
    MYSQL* conRALL;
    connection_pool* poolRALL;
};

#endif