## 第五章 优化程序性能

> 编译器必须很小心地对程序只使用安全的优化,也就是说对于程序可能遇到的所有可能的情况,在C语言标准提供的保证之下,优化后得到的程序和未优化的版本有一样的行为.

#### 5.1 优化编译器的能力和局限性

* 两个指针可能指向同一个内存位置的情况称为内存别名使用.

例如:

```
x=1000;y=3000;

*q=y; /*3000*/

*p=x; /*1000*/

t1=*q; /*1000 or 3000*/
```

如果编译器不能确定两个指针是否指向同一个位置,就必须假设什么情况都有可能,这就限制了可能的优化策略.

##### 练习题5.1

如果xp=yp,则无论传入参数值是什么,都会清零.

* 第二个妨碍优化的因素是函数调用.

假设函数有副作用,修改了全局程序状态的一部分.

大多数编译器不会试图判断一个函数是否没有副作用,如果没有,就可能被优化成像fun2中的样子.相反,编译器会假设最糟的情况,并保持所有的函数调用不变.

#### 5.2 表示程序性能

引入量度标准`每元素的周期数`(CPE),作为一种表示程序性能并知道我们改进代码的方法.

提到了`循环展开`的一个技术.大概意思就是在一个循环进行更多的迭代.

#### 5.4 消除循环的低效率

例如书上的例子:

```
#define IDENT 0
#define OP +

void combine1(vec_ptr v,data_t *dest){
    long i;
    *dest = IDENT;

    for (i = 0; i < vec_length(v);i++) {
        data_t val;
        //读取第i的索引的值到val中
        get_vec_element(v, i, &val);
        //将val根据OP累计到*dest中
        *dest = *dest OP val;
    }
}
```

里面循环的vec_length(v)移动到循环外.

```
void combine2(vec_ptr v,data_t *dest){
    //用局部变量存储向量长度
    long length = vec_length(v);

    long i;
    *dest = IDENT;

    for (i = 0; i < length;i++) {
        data_t val;
        get_vec_element(v, i, &val);
        *dest = *dest OP val;
    }
}
```

效率就提高了不少.

这个优化是一类常见的优化的一个例子,称为`代码移动`.这类优化包括识别要执行多次(例如在循环中)但是计算结果不会改变的计算.

这个示例说明了编程时一个常见的问题,**一个看上去无足轻重的代码片段有隐藏的渐进低效率.**

#### 5.5 减少调用过程

```
void combine3(vec_ptr v, data_t *dest) {
    long i;
    long length = vec_length(v);
    data_t *data = get_vec_start(v);
    //直接获取数组的首元素指针, 不再调用函数
    *dest=IDENT;
    for (i = 0; i < length; i++) {
        *dest = *dest OP data[i];
    }
}
```

这里书上提到了combine3把get_vec_element()函数调用提出来了,但是代码没有显示性能提升.

#### 5.6 消除不必要的内存引用

在这里解答上面的问题.

每次迭代时,积累变量的数值都要从内存读出再写入到内存.这样的读写很浪费,因为每次迭代开始时从dest读出来的值就是上次迭代最后写入的值.

```
void combine4(vec_ptr v, data_t *dest) {
    long i;
    long length = vec_length(v);
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;
    for (i = 0; i < length; i++) {
        acc = acc OP ptr[i];
    }
    *dest = acc;
}
```

为了消除这种不必要的内存读写,combine4引入了一个临时变量来存储.性能有了明显提高.