#include "serializer.h"

int main() {
  {
    Base1 a1(2);
    Serialized s1;
    s1.type = 0;
    s1.obj = &a1;

    Base1 a2(3);
    Serialized s2;
    s2.type = 0;
    s2.obj = &a2;

    Base2 b1(4);
    Serialized s3;
    s3.type = 1;
    s3.obj = &b1;

    Base2 b2(5);
    Serialized s4;
    s4.type = 1;
    s4.obj = &b2;

    vector<Serialized> v;
    v.push_back(s1);
    v.push_back(s2);
    v.push_back(s3);
    v.push_back(s4);

    Serializer s;
    s.Serialize("data", v);
  }

  {
    Serializer s;
    vector<Serialized> v;
    s.Deserialize("data", v);

    for (int i = 0; i < v.size(); i++) {
      if (v[i].type == 0) {
        Base1 *p = (Base1 *)(v[i].obj);
        p->Print();
      } else if (v[i].type == 1) {
        Base2 *p = (Base2 *)(v[i].obj);
        p->Print();
      }
    }
  }
  return 0;
}