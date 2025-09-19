//
// Created by Administrator on 2025/9/19.
//
#include <iostream>
#include <string>
using namespace std;

// 类型模板
template<typename T>
class Person {
public:
    T name;
    void print() {
        cout << name << endl;
    }
};

// 某个具体数据的模板
template<size_t kMaxStackSize>
class aClass {
public:
    size_t getSize() const { return kMaxStackSize; }
    size_t m_size = kMaxStackSize;
};
int main() {
    Person<string> p{"asd"};
    p.print();


    aClass<121323123> a;
    cout << a.getSize() << endl;
}