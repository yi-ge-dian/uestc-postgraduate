#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <vector>

// 序列化接口
class ILSerializable {
public:
  virtual bool Serialize(FILE *fp) = 0;
  virtual ILSerializable *Deserialize(FILE *fp) = 0;
  virtual bool GetType(int &type) = 0;
};

// 类 A
class A : public ILSerializable {
public:
  A() { i = 0; }
  explicit A(int j) { i = j; }
  virtual ~A() {}

public:
  void f() { std::cout << "in f(): " << i << std::endl; }

  virtual bool GetType(int &type) {
    type = 0;
    return true;
  }

  virtual bool Serialize(FILE *fp) {
    if (fp == NULL)
      return false;
    fwrite(&i, sizeof(int), 1, fp);
    return true;
  }

  virtual ILSerializable *Deserialize(FILE *fp) {
    A *p = new A();
    fread(&(p->i), sizeof(int), 1, fp);
    return p;
  }

private:
  int i;
};

// 类B
class B : public ILSerializable {
public:
  B() {
    i = 0;
    j = 0;
  }

  explicit B(int k) {
    i = k;
    j = k + 1;
  }

  virtual ~B() {}

public:
  void f() { std::cout << "in f(): " << i << " " << j << std::endl; }

public:
  virtual bool GetType(int &type) {
    type = 1;
    return true;
  }

  virtual bool Serialize(FILE *fp) {
    if (fp == NULL)
      return false;
    fwrite(&i, sizeof(int), 1, fp);
    fwrite(&j, sizeof(int), 1, fp);
    return true;
  }

  virtual ILSerializable *Deserialize(FILE *fp) {

    B *p = new B();
    fread(&(p->i), sizeof(int), 1, fp);
    fread(&(p->j), sizeof(int), 1, fp);
    return p;
  }

private:
  int i;
  int j;
};