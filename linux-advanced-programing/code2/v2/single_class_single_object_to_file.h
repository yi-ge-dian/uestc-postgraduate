#pragma once

#include <fstream>
#include <iostream>
#include <string>

using namespace std;
class Base {

public:
  // 构造函数
  Base() { value_ = -1; }

  // 有参构造
  Base(int value) { value_ = value; }

  // 析构函数
  ~Base(){};

  // 序列化
  bool Serialize(const char *buffer) {
    auto length = sizeof(this);
    cout << "length of buffer" << length << endl;
    // 写文件
    ofstream os(buffer);
    os.write((char *)this, length);
    os.close();
    return true;
  }

  // 反序列化
  static Base Deserialize(const char *buffer) {
    char buf[1000];
    // 从文件中读取内容
    ifstream is(buffer);
    is >> buf;
    Base *a = (Base *)buf;
    is.close();
    return (*a);
  }

  // 打印
  void print() { cout << "value = " << value_ << endl; }

private:
  int value_;
};