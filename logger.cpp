#include<iostream>

#include"logger.hpp"
#include"Timestamp.hpp"

Logger& Logger::instance(){
    static Logger logger;
    return logger;
};

//设置日志级别
void Logger::setLogLevel(LogLevel level){
   logLevel_=level;
};

//获取日志级别
LogLevel Logger::logLevel(){
    return logLevel_;
}

//写日志
void Logger::log(std::string msg){
    switch (logLevel_)
    {
    case INFO:
        std::cout<<"[INFO]";
        break;
    case ERROR:
        std::cout<<"[ERROR]";
        break; 
    case FATAL:
        std::cout<<"[FATAL]";
        break; 
    case DEBUG:
        std::cout<<"[DEBUG]";
        break; 
    default:
        break;
    }

    std::cout<<" time:"<<Timestamp::now().toString()<<" : "<<msg<<std::endl;
}