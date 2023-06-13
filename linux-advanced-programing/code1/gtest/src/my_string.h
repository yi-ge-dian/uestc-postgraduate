#include <string.h>

class MyString {
 private:
  const char *c_string_;
  const MyString &operator=(const MyString &rhs);

 public:
  // 默认构造函数
  MyString() : c_string_(nullptr) {}

  // 构造函数，禁止隐式转换
  explicit MyString(const char *a_c_string) : c_string_(nullptr) { Set(a_c_string); }

  // 拷贝构造函数
  MyString(const MyString &string) : c_string_(nullptr) { Set(string.c_string_); }

  // 析构函数
  ~MyString() { delete[] c_string_; }

  const char *c_string() const { return c_string_; }
  size_t Length() const { return c_string_ == nullptr ? 0 : strlen(c_string_); }

  static const char *CloneCString(const char *a_c_string);
  void Set(const char *c_string);
};