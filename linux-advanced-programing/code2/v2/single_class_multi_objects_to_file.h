#pragma once

#include "single_class_single_object_to_file.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>

using namespace std;

class LoopBase{

public:
    // 序列化
    static bool Serialize(const char* file_path, vector<Base *>& v){
        // 读写方式打开文件
        FILE* fp = fopen(file_path, "w+");
        // 遍历 vector 中所有的元素，写入文件中
        for(int i = 0; i < v.size(); i++){
            fwrite(v[i], sizeof(int), 1, fp);
        }
        fclose(fp);
        return true;
    }

    static int Deserialize(const char* file_path, vector<Base *>& v){
        // 读的方式打开文件
        FILE *fp = fopen(file_path, "r");
        // 把文件中的数据读到 vector 中
        for(int i = 0; i < v.size(); i++){
            fread(v[i], sizeof(int), 1, fp);
        }
        fclose(fp);
        return 1;
    }

};