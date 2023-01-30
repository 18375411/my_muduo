#pragma once

#include<string>

#include"noncopyable.hpp"

//LOG_INFO("%s %d",arg1,arg2)
#define LOG_INFO(Logmsgformat,...)\
do\
{\
    Logger &logger=Logger::instance();\
    logger.setLogLevel(INFO);\
    char buf[1024]={0};\
    snprintf(buf,1024,Logmsgformat,##_VA_ARGS__);\
}while(0)\

#define LOG_ERROR(Logmsgformat,...)\
do\
{\
    Logger &logger=Logger::instance();\
    logger.setLogLevel(ERROR);\
    char buf[1024]={0};\
    snprintf(buf,1024,Logmsgformat,##_VA_ARGS__);\
}while(0)\

#define LOG_FATAL(Logmsgformat,...)\
do\
{\
    Logger &logger=Logger::instance();\
    logger.setLogLevel(FATAL);\
    char buf[1024]={0};\
    snprintf(buf,1024,Logmsgformat,##_VA_ARGS__);\
}while(0)\

#ifdef MUDEBUG
#define LOG_DEBUG(Logmsgformat,...)\
    do\
    {\
        Logger &logger=Logger::instance();\
        logger.setLogLevel(DEBUG);\
        char buf[1024]={0};\
        snprintf(buf,1024,Logmsgformat,##_VA_ARGS__);\
    }while(0)\
#else
    #define LOG_DEBUG(Logmsgformat,...)
#endif

//定义日志级别 INFO DEBUG EORROR FATAL
enum LogLevel{
    INFO,//普通信息
    ERROR,//错误信息
    FATAL,//致命信息
    DEBUG//调试信息
};

//单例模式
class Logger:noncopybale{
public:
    //获取唯一单例
    static Logger& instance();
    //设置日志级别
    void setLogLevel(LogLevel level);
    //获取日志级别
    LogLevel logLevel();

    //写日志
    void log(std::string msg);
    

private:
    Logger()=default;

    LogLevel logLevel_;
};
