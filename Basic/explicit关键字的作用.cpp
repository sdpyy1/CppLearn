#include <iostream> 
class aClass{
public:
    int a;
    explicit aClass(int val) : a(val) {}
};

void func(aClass obj){
    std::cout << "Value: " << obj.a << std::endl;
}

int main(){
    func(10);  // 这里会报错，因为不能隐式把 int 转成 aClass。  当类作为函数参数时，必须传入类，而不是类参数
    return 0;
}
