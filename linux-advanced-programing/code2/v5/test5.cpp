#include "serializer.h"

int main() {
  FILE *fp = fopen("config.txt", "r+");
  char buf[100];
  int len;
  while (fgets(buf, 100, fp) != NULL)
    ;
  {
    A a1(2);
    B b1(3);
    B b2(4);
    A a2(5);

    std::vector<ILSerializable *> v;

    v.push_back(&a1);
    v.push_back(&b1);
    v.push_back(&b2);
    v.push_back(&a2);

    CLSerializer s;
    s.Serialize(buf, v);
  }

  {
    CLSerializer s;
    A a;
    B b;
    s.Register(&a);
    s.Register(&b);

    std::vector<ILSerializable *> v;
    s.Deserialize(buf, v);

    for (int i = 0; i < v.size(); i++) {
      A *p = dynamic_cast<A *>(v[i]);
      if (p != NULL)
        p->f();

      B *q = dynamic_cast<B *>(v[i]);
      if (q != NULL)
        q->f();
    }
  }

  return 0;
}