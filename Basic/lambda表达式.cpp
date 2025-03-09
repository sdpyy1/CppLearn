// 匿名函数也被称为 lambda 表达式，它是一种在需要的地方定义和使用的小型、一次性的函数对象。以下从语法、使用场景、捕获列表等多个方面详细介绍 C++ 匿名函数。
/**
 * [capture list] (parameter list) mutable(可选) exception(可选) -> return type(可选) {
    // 函数体
    }
    capture list: 用于指定 lambda 表达式可以访问的外部变量
    mutable: 默认情况下捕获的外部变量只读，如果想修改需要加这个参数
 */
//int main(){
//    int x = 10;
//    int y = 20;
//    auto add = [x,y]() mutable->int{
//        x=1;
//        return x+y;
//    };
//    int ret = add();
//}