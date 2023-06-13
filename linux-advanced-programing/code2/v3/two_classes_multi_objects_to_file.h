#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace std;

class Base1 {
public:
  Base1() { value_ = 0; }

  Base1(int v) : value_(v) {}

  void Print() { cout << "value = " << value_ << endl; }

public:
  // 序列号
  bool Serialize(FILE *fp) {
    if (fp == NULL) {
      return false;
    }
    fwrite(&value_, sizeof(int), 1, fp);
    return true;
  }
  // 反序列化
  bool Deserialize(FILE *fp) {
    if (fp == NULL) {
      return false;
    }
    fread(this, sizeof(int), 1, fp);
    return true;
  }

private:
  int value_;
};

class Base2 {
public:
  Base2() {
    value_1 = 0;
    value_2 = 0;
  }

  Base2(int v) : value_1(v), value_2(v + 1) {}

  void Print() { cout << "value_1 = " << value_1 << " , value_2 = " << value_2 << endl; }
  // 序列化
  bool Serialize(FILE *fp) {
    if (fp == NULL) {
      return false;
    }
    fwrite(&value_1, sizeof(int), 1, fp);
    fwrite(&value_2, sizeof(int), 1, fp);
    return true;
  }
  // 反序列化
  bool Deserialize(FILE *fp) {
    fread(&(this->value_1), sizeof(int), 1, fp);
    fread(&(this->value_2), sizeof(int), 1, fp);
    return true;
  }

private:
  int value_1;
  int value_2;
};