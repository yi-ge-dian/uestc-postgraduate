#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "food.h"
#include "../../others/json/json.h"

using namespace std;

class Server {
public:
    // 服务器名字，IP，端口
    string name_; 
    string ip_;   
    int port_;
    // 标志
    int flag_;  // 是否有新的内容写入文件了
    int flag2_; // 是否另外一个服务器发生故障了
    // 菜单数据转成的字符串
    string data_str_;
    // 菜单数据
    vector<Food> foods_;
    // 构造函数
    Server();
    Server(const string name, string datafile);
    // 析构函数
    ~Server();
    void addFood();
    void deleteFood();
    void updateFood();
    void show();
    void saveData();
    void writeFileFromString(const string &filename, const string &body);

private:

    //读取json文件返回json对像
    Json::Value readJsonFile(const string &filename);

    //把json对像写入文件
    void writeJsonFile(const string &filename, const Json::Value &root);
};