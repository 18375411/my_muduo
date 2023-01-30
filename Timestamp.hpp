#pragma once

#include<iostream>
#include<string>

class Timestamp{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);//取消隐式转换

    static Timestamp now();
    std::string toString() const;//const表明该成员函数无法修改成员变量
private:
    int64_t microSecondsSinceEpoch_;
};