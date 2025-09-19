#include <cstdint>
#include <bitset>
#include "iostream"
#include "string"
using namespace std;

// 打印任意类型的内存二进制表示
template <typename T>
void printMemoryBinary(const T& value) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&value);
    const size_t size = sizeof(T);

    for (size_t i = 0; i < size; ++i) {
        // 打印每个字节的8位二进制
        for (int bit = 7; bit >= 0; --bit) {
            std::cout << ((bytes[i] >> bit) & 1);
        }
        std::cout << " "; // 字节之间加空格
    }
    std::cout << std::endl;
}

// 方法2：使用标准库bitset（适用于已知大小的整数类型）
void printBinaryWithBitset(std::uint32_t value) {
    std::cout << std::bitset<32>(value) << std::endl;
}
struct aStruct {
    char b;
    int a;
};
class aClass {
public:
    char b;
    int a;
    void print() {
        cout<< a<<b<<endl;
    }
};
int main() {
    int a = 1;
    // sizeof的使用
    cout << sizeof(char)<<endl;
    cout << sizeof(int)<<endl;
    cout << sizeof(float)<<endl;
    cout << sizeof(double)<<endl;
    aClass b{1,1};
    cout << sizeof(b)<<endl;
    printMemoryBinary<aClass>(b);
}
