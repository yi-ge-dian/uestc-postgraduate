#include "two_classes_multi_objects_to_file.h"

struct Serialized {
  int type;
  void *obj;
};

class Serializer {
public:
  bool Serialize(const char *file_path, vector<Serialized> &v) {
    FILE *fp = fopen(file_path, "w+");
    // 打开失败
    if (fp == NULL) {
      return false;
    }
    // 遍历 vector 中的元素
    for (int i = 0; i < v.size(); i++) {
      fwrite(&(v[i].type), sizeof(int), 1, fp);
      // 根据 vector 中元素的类型还序列化
      if (0 == v[i].type) {
        Base1 *p = (Base1 *)(v[i].obj);
        p->Serialize(fp);
      } else if (1 == v[i].type) {
        Base2 *p = (Base2 *)(v[i].obj);
        p->Serialize(fp);
      }
    }
    fclose(fp);
    return true;
  }

  bool Deserialize(const char *file_path, vector<Serialized> &v) {
    FILE *fp = fopen(file_path, "r+");
    // 不停的遍历
    for (;;) {
      int type;
      // 读取
      int r = fread(&type, sizeof(int), 1, fp);
      if (-1 == r || 0 == r)
        break;
      // 根据读出来的类型进行反序列化
      if (0 == type) {
        Base1 *p;
        p = new Base1();
        p->Deserialize(fp);

        Serialized s;
        s.type = type;
        s.obj = p;
        v.push_back(s);
      } else if (1 == type) {
        Base2 *p;
        p = new Base2();
        p->Deserialize(fp);

        Serialized s;
        s.type = type;
        s.obj = p;
        v.push_back(s);
      }
    }
    fclose(fp);
    return true;
  }
};