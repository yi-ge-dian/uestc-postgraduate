#include "multi_classes_multi_objects_to_file.h"

class CLSerializer {
public:
  bool Serialize(const char *pFilePath, std::vector<ILSerializable *> &v) {
    FILE *fp = fopen(pFilePath, "w+");
    if (fp == NULL)
      return false;
    for (int i = 0; i < v.size(); i++) {
      int type;
      v[i]->GetType(type);
      fwrite(&type, sizeof(int), 1, fp);
      v[i]->Serialize(fp);
    }
    fclose(fp);
    return true;
  }

  bool Deserialize(const char *pFilePath, std::vector<ILSerializable *> &v) {
    FILE *fp = fopen(pFilePath, "r+");
    if (fp == NULL)
      return false;
    for (;;) {
      int nType = -1;
      int r = fread(&nType, sizeof(int), 1, fp);

      int type;
      for (int i = 0; i < m_vSerialized.size(); i++) {
        m_vSerialized[i]->GetType(type);
        if (type == nType) {
          ILSerializable *p = m_vSerialized[i]->Deserialize(fp);
          if (p != NULL)
            v.push_back(p);
        }
      }
      if (r == 0)
        break;
    }
    fclose(fp);
    return true;
  }


  void Register(ILSerializable *pSerialized) {
    m_vSerialized.push_back(pSerialized);
  }

private:
  std::vector<ILSerializable *> m_vSerialized;
};