#include "sql_connection_pool.h"
#include "locker.h"

using namespace std;

connection_pool::connection_pool()
{
    m_CurConn = 0;
    m_FreeConn = 0;
}

connection_pool* connection_pool::GetInstance()
{
    static connection_pool connPool;
    return &connPool;
}

void connection_pool::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn, int close_log)
{
    m_url = url;
    m_port = Port;
    m_user = User;
    m_passWord = PassWord;
    m_databaseName = DBName;
    m_close_log = close_log;

    for(int i = 0; i < MaxConn; i++) {
        MYSQL* con = NULL;
        con = mysql_init(con);

        if(con == NULL) {
            std::cerr << "MySQL Error. " << std::endl;
            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);
        if(con == NULL) {
            std::cerr << "MySQL error. " << std::endl;
            exit(1);
        }

        m_connList.push_back(con);
        ++m_FreeConn;
    }
    m_reserve = sem(m_FreeConn);

    m_MaxConn = m_FreeConn;
}

// 收到请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL* connection_pool::GetConnection()
{
    MYSQL* con = NULL;

    if(m_connList.size() == 0) {
        return NULL;
    }

    m_reserve.wait();

    lock.lock();

    con = m_connList.front();
    m_connList.pop_front();

    --m_FreeConn;
    ++m_CurConn;

    lock.unlock();

    return con;
}

// 释放使用的连接
bool connection_pool::ReleaseConnection(MYSQL* con)
{
    if(con == NULL) {
        return false;
    }

    lock.lock();
    m_connList.push_back(con);
    ++m_FreeConn;
    --m_CurConn;

    lock.unlock();

    m_reserve.post();
    return true;
}

// 销毁数据库连接池
void connection_pool::DestoryPool()
{
    lock.lock();
    if(m_connList.size() > 0) {
        list<MYSQL*>::iterator itr;
        for(itr = m_connList.begin(); itr != m_connList.end(); ++itr) {
            MYSQL *con = *itr;
            mysql_close(con);
        }

        m_CurConn = 0;
        m_FreeConn = 0;
        m_connList.clear();
    }
    lock.unlock();
}

// 当前空闲的连接数
int connection_pool::GetFreeConn()
{
    return this->m_FreeConn;
}

connection_pool::~connection_pool()
{
    DestoryPool();
}


connectionRALL::connectionRALL(MYSQL** SQL, connection_pool *connPool)
{
    *SQL = connPool->GetConnection();
    conRALL = *SQL;
    poolRALL = connPool;
}

connectionRALL::~connectionRALL()
{
    poolRALL->ReleaseConnection(conRALL);
}