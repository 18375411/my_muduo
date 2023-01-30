#pragma once
/**
 * 派生类对象无法进行拷贝构造和赋值操作
*/
class noncopybale{
public:
    noncopybale(const noncopybale&)=delete;
    void operator=(const noncopybale&)=delete;
protected:
    noncopybale()=default;
    ~noncopybale()=default;
};