#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
using namespace std;

int main(){
    cout << "begin" << endl;
    if (fork()!=0){
        cout<< "i am the parent process, pid = " << getpid() << ", ppid = " << getppid()<<endl;
        for (int i = 0; i < 10; i++){
            cout <<" i = " << i <<  endl;
            sleep(1);
        }
        exit(0);
    }else{
        cout<< "i am the child process, pid = " << getpid() << ",   ppid = " << getppid()<<endl;
        for (int j = 0; j < 10; j++){
            cout <<" j = " << j <<  endl;
            sleep(1);
        }
        exit(0);
    }
    return 0;
}

    
