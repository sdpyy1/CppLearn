//
// Created by Administrator on 2025/3/9.
//
#include "iostream"
using namespace std;
int add(int a,int b){
    return a+b;
}

int main(){
    // 声明一个函数指针（需要声明参数类型和返回值类型）用来指向一个函数
    int (*funPointer)(int,int);
    funPointer = add; // 指向
    int result = funPointer(1,2);
    cout << result << endl;
    return 0;
}