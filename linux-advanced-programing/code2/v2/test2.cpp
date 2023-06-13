#include "single_class_multi_objects_to_file.h"
#include "single_class_single_object_to_file.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
  // 待序列化的数据
  Base a1(100);
  Base a2(200);

  vector<Base *> v1;
  v1.push_back(&a1);
  v1.push_back(&a2);

  LoopBase::Serialize("data2", v1);

  // 待被反序列化的数据
  Base a3;
  Base a4;

  vector<Base *> v2;
  v2.push_back(&a3);
  v2.push_back(&a4);
  LoopBase::Deserialize("data2", v2);

  // 打印值
  a3.print();
  a4.print();
  return 0;
}