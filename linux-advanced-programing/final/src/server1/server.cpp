#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"
#include "food.h"
#include "../../others/json/json.h"
#include "worker.h"

using namespace std;

// 从文件读取菜单数据
Json::Value Server::readJsonFile(const string &filename) {
    // 1. 打开文件
    ifstream ifile;
    ifile.open(filename);

    // 2. 创建json读工厂对象
    Json::CharReaderBuilder ReaderBuilder;
    ReaderBuilder["emitUTF8"] = true; // utf8支持，不加这句，utf8的中文字符会编程\uxxx

    // 3. 创建json对象，等会要返回这个对象
    Json::Value root;

    // 4. 把文件转变为json对象，要用到上面的三个变量,数据写入root
    string strerr;
    bool ok = Json::parseFromStream(ReaderBuilder, ifile, &root, &strerr);
    if (!ok) {
        cerr << "json解析错误";
    }
    // 5. 返回存有数据的json，这个json对象已经能用了

    if (root.isNull()) {
        // cout << "是空的";
    } else {
        // cout << "有内容" << endl;
        for (int i = 0; i < root.size(); i++) {
            Food food;
            // cout << root[i] << endl;
            food.price = root[i]["price"].asFloat();
            food.name = root[i]["name"].asString();
            food.stock = root[i]["stock"].asInt();
            this->foods_.push_back(food);
        }
    }
    ifile.close();
    return root;
}

// 保存数据
void Server::saveData() {
    Json::Value root;

    for (int i = 0; i < this->foods_.size(); i++) {
        Json::Value item;
        item["name"] = this->foods_[i].name;
        item["price"] = this->foods_[i].price;
        item["stock"] = this->foods_[i].stock;
        root[i] = item;
    }
    this->writeJsonFile("data.json", root);
}

// 把json对像写入文件
void Server::writeJsonFile(const string &filename, const Json::Value &root) {
    flag_ = 1;
    this->data_str_ = root.toStyledString();

    Json::StyledWriter writer;
    ofstream os;
    cout << "保存" << endl;
    os.open(filename);
    os << writer.write(root);
    os.close();
}

// 把菜单字符串写入文件
void Server::writeFileFromString(const string &filename, const string &body) {
    // cout << filename << endl;
    ofstream ofile(filename);
    ofile << body;
    ofile.close();
}

Server::Server() {
}

Server::Server(const string name, string datafile = "") {
    Json::Value value;
    if (datafile == "") {
        this->readJsonFile("data.json");
    } else {
        this->readJsonFile("another-server.json");
    }

    // 名字，IP，端口，
    name_ = name;
    ip_ = "127.0.0.1";
    if (name_ == "A") {
        port_ = 11111;
    }
    if (name_ == "B") {
        port_ = 22222;
    }
    if (datafile != "") {
        port_ = 61111;
    }

    // 标志
    flag_ = 0;
    flag2_ = 0;

    // 菜单字符串
    data_str_ = "";

    // 启动线程1：点单线程
    static pthread_t pid;
    int ret = pthread_create(&pid, NULL, order, this);
    if (ret != 0) {
        cout << "pthread_create error order" << endl;
        return;
    }

    // 启动线程2：接收数据，线程3：发送数据，用来同步两个服务器之间的数据
    if (datafile == "") {
        ret = pthread_create(&pid, NULL, recvData, this);
        if (ret != 0) {
            cout << "pthread_create error recvData" << endl;
            return;
        }

        ret = pthread_create(&pid, NULL, sendData, this);
        if (ret != 0) {
            cout << "pthread_create error sendData" << endl;
            return;
        }
    }

    cout << "服务器" << name << "启动成功" << endl;
}

Server::~Server() {
}

// 添加一个食物
void Server::addFood() {
    cout << "添加一个食物" << endl;

    Food food;
    cout << "请输入食物名字" << endl;
    cin >> food.name;
    cout << "请输入价钱" << endl;
    cin >> food.price;
    cout << "请输入库存" << endl;
    cin >> food.stock;
    this->foods_.push_back(food);
}

// 删除一个食物
void Server::deleteFood() {
    if (foods_.size() < 1) {
        return;
    }

    cout << "删除一个食物" << endl;

    for (int i = 0; i < this->foods_.size(); i++) {
        cout << i << ":" << this->foods_[i].name << " 价钱:" << this->foods_[i].price << " 库存:" << this->foods_[i].stock << endl;
    }

    cout << "请输入要删除的序号:" << endl;
    int index;
    cin >> index;
    this->foods_.erase(this->foods_.begin() + index);
}

//修改一个食物
void Server::updateFood() {
    cout << "修改一个食物" << endl;

    cout << "请输入要修改的序号:";
    int index;
    cin >> index;
    cout << endl
         << "请输入修改后的名字:";
    string name;
    cin >> name;
    cout << endl
         << "请输入修改后的价钱:";
    float price;
    cin >> price;
    cout << endl
         << "请输入修改后的库存:";
    float stock;
    cin >> stock;
    foods_[index].name = name;
    foods_[index].price = price;
    foods_[index].stock = stock;
}

//显示所有食物
void Server::show() {
    for (int i = 0; i < this->foods_.size(); i++) {
        cout << i << ":" << this->foods_[i].name << " 价钱:" << this->foods_[i].price << " 库存:" << this->foods_[i].stock << endl;
    }
    cout << "请按任意键继续\n"
         << endl;
    cin.get();
    cin.get();
}