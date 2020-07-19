## 第三章 程序的机器级表示

#### 3.2.1 机器级代码

* 程序计数器(通常称为"PC",在x86-64中用%rip表示)给出将要执行的下一条指令在内存中的地址.
* 整数寄存器文件中包括16个命名的位置,分别存储64位的值.这些寄存器可以存储地址或整数数据
* 一组向量寄存器可以存放一个或多个整数或浮点数值

程序内存包含:程序的可执行机器代码,操作系统需要的一些信息,用来管理过程调用和返回的运行时栈,以及用户分配的内存块(比如说用malloc库函数分配的)



#### 3.4.1 操作数指示符

大多数指令有一个或多个操作数,指示出执行一个操作中要使用的源数据值,以及放置结果的目的位置.

各种不同的操作数的可能性被分为三种类型:

1. 立即值, 用来表示常数, 用$开头.可以直接写整数,汇编器会将其编译成对应的二进制格式.
2. 寄存器, 即寄存器的名称, 代表某个寄存器内部的值.
3. 内存引用, 代表某个内存地址的值, 也就是寻址或者说是取地址运算, 用括号包起来一个值, 这个值可以是常量, 取得的值或者是计算所得.

寻址的模式,最常用的形式是 

`计算出的内存地址 = 立即数偏移量 + Rb + Ri * s`



##### 练习题3.1

|      地址      |  值  |
| :------------: | :--: |
| 内存地址 0x100 | 0xFF |
| 内存地址 0x104 | 0xAB |
| 内存地址 0x108 | 0x13 |
| 内存地址 0x10C | 0x11 |

| 寄存器 |  值   |
| :----: | :---: |
|  %rax  | 0x100 |
|  %rcx  |  0x1  |
|  %rdx  |  0x3  |



|     操作数      |                              值                              |
| :-------------: | :----------------------------------------------------------: |
|      %rax       |                            0x100                             |
|      0x104      |                  表示寻绝对地址,结果是0xAB                   |
|     $0x108      |                          常量 0x108                          |
|     (%rax)      |        将%rax寄存器中的0x100当做地址寻址,结果是 0xFF         |
|     4(%rax)     | 表示基址+偏移量地址,为 %rax中的 0x100地址加0x4,即 0x104地址,结果为0xAB |
|  9(%rax, %rdx)  |    表示变址寻址, 为 9+ 0x100 + 0x3 = 0x10C, 所以值是0x11     |
| 260(%rcx, %rdx) |           同理地址是 260 + 1 + 3 = 0x108, 值是0x13           |
|  0xFC(,%rcx,4)  |       比例变址寻址,结果是 0xFC + 4 = 0x100, 值是 0xFF        |
| (%rax, %rdx, 4) |  带s的变址寻址,计算后的内存是 0x100 + 4*3 = 0x10C,值是0x11   |

#### 3.4.2 数据传送指令

##### MOV类

> 指令为: MOV  S , D
>
> 效果为: 传送 D<-S

在这里把S叫做源操作数, 把D叫做目的操作数.

MOV类由四条指令构成:**movb**,**movw**,**movl**,**movq**,区别是操作数大小不同.

x86-64限制了MOV的两个操作数都不能都是内存地址,如果要实现,就找寄存器当工具人. 

大多数情况中,MOV指令只会更新目的操作数指定的那些寄存器字节或内存位置.唯一例外是,**movl**指令以寄存器作为目的时,会把该寄存器的高位4字节设置为0.这是由于x86-64采用的惯例,即任何为寄存器生成的32位值得指令都会把该寄存器的高位部分置为0.

常规的movq命令只能处理32位的源操作数,将其符号扩展得到64位. 而movbsq可以直接将64位数作为源操作数, 但目的只能是寄存器.

关于MOVZ和MOVS,这两个指令都是在将较小的源值复制到较大的目的时使用.

区别在于MOVZ使用0来填充,而MOVS使用符号位来填充.

在MOVS中有个指令cltq,用于将%eax符号扩展到%rax,等价于movslq %eax,%rax.



##### 练习题3.3

`movb $0xF, (%ebx)`	内存引用的寄存器必须是4bytes，改成`movb $0xF, (%rbx)`
`movb %al, %sl`	没有叫做`sl`的寄存器,只有`sil`和`spl`.
`movl %eax, %rdx`	寄存器大小不匹配



##### 练习题3.4

|     src_t     |  dest_t  |                             指令                             |
| :-----------: | :------: | :----------------------------------------------------------: |
|     long      |   long   |          movq (%rdi),%rax; <br />move %rax, (%rsi)           |
|     char      |   int    | 扩展符号到32位,<br />movsbl (%rdi), %eax;<br />movl %eax, (%rsi) |
|     char      | unsigned |         movsbl (%rdi), %eax;<br />movl %eax, (%rsi)          |
| unsigned char |   long   | unsigned, 零扩展. 零扩展没有扩展到64位的指令,32位就相当于64位的零扩展了.<br />movzbl (%rdi), %rax<br />movq %rax, (%rsi) |
|      int      |   char   |  区别在第二步<br />movl (%rdi), %eax<br />movb %al, (%rsi)   |



##### 练习题3.5

不考虑编译优化的话是:

```
void decode1(long *xp, long *yp, long *zp){
	long temp1 = *xp;
	long temp2 = *yp;
	long temp3 = *zp;
	*yp = temp1;
	*zp = temp2;
	*xp = temp3;
}
```



#### 3.4.4 压入和弹出栈数据

从%rbp寄存器中压一个四字入栈的指令`pushq`可以分解为:

```popq可以分解为:
subq $8, %rsp
push %rbp, (%rsp)
```

`popq`可以分解为:

```
movq (%rsp), %rax
addq %$8, %rsp
```



#### 3.5.3 移位操作

移位量可以是一个立即数,或者放在单字节寄存器%cl上.

| leaq 7 (%rax, %rax, 8), %rdx | `x + 8x + 7` |
| ---------------------------- | ------------ |
|                              |              |

##### 练习3.9

```
// x in %rdi, n in %rsi
shift_left4_rightn:
    movq %rdi, %rax;
    salq  $4, %rax     # x <<= 4
    movq %esi, %ecx;
    sarq  %cl, %rax    # x >>= n
```

##### 练习3.10

```
long t1 = x | y;
long t2 = t1 >> 3;
long t3 = ~t2;
long t4 = z - t3;
```

##### 练习3.11:

用于置0.比move开销小.



#### 3.5.5 特殊的算术操作

`imulq`和`mulqs`: "单操作数"乘法指令,用于处理128位的乘积.其中一个参数必须在%rax中.结果用%rdx存高64位,用%rax存低64位.

`idivq`:这个是有符号数的除法指令, 操作数S是除数, 被除数固定使用%rax作为低64字节, %rdx作为高64字节. 除法进行完之后,商存储在%rax中, 而余数存储在%rdx中.

`cqto`: 这个指令没有操作数, 将%rdx的位设置为%rax的符号位



##### 练习3.12

把`cqto`改成把%rdx的位置为0;



#### 3.6.1条件码

1. CF 进位标志, 最近的操作使最高位产生进位
2. ZF 零标志, 最近的操作的结果得到0
3. SF 符号标志, 最近的操作结果是负数
4. OF 溢出标志, 最近的操作导致补码溢出,正负溢出都算



`CMP`和`TEST`只改变条件码,前者是减法,后者是与(检测正负0).

同时注意`CMP`的操作数与`SUB`是相反的.

`SET`和`JMP`都可以根据条件码进行判断操作.

##### 练习3.13

- ```
  cmpl  %esi, %edi
  setl  %al
  ```

  后缀 ‘l’ 和寄存器指示符表明是 32 位操作数，而且调用的是有符号的小于比较。所以 `data_t` 一定是 int 。

- ```
  cmpw  %si, %di
  setge %al
  ```

  后缀 ‘w’ 和寄存器指示符表明是 16 位操作数，而且调用的是有符号的大于等于。所以 `data_t` 一定是 short。

- ```
  cmpb  %sil, %dil
  setbe %al
  ```

  后缀 ‘b’ 和寄存器指示符表明是 8 位操作数，而且调用的是无符号小于等于。所以 `data_t` 一定是 unsigned char 。

- ```
  cmpq  %rsi, %rdi
  setne %al
  ```

  后缀 ‘q’ 和寄存器指示符表明是 64 位操作数，同时比较符号是 `!=` ，有符号、无符号和指针参数都是一样的。所以 `data_t` 可以是 long、unsigned long 或 char * 。



`JMP`三种用法:

1. `JMP .L1`  等于goto
2. `JMP *%rax ` 用%rax的值作为跳转目标

3. `JMP *(%rax) `以%rax的值来读地址,从内存中读出并且跳转.



##### 练习3.15

```
4003fa: 74 02    je    XXXXXX
4003fc: ff do    callq *%rax
```

> je 指令的目标为 `0x4003fc + 0x02 = 0x4003fe` 

```
40042f: 74 f4    je   XXXXXX
400431: 5d       pop  %rbp
```

> je 指令的目标为 `0x400431 - 12（0xf4是-12的一个字节的补码表示）= 0x400425` 

```
XXXXXX: 77 02    ja   400547
XXXXXX: 5d       pop  %rbp
```

> 跳转目标是绝对地址 `0x400547` 。根据字节编码，一定在距离 pop 指令 `0x2` 的地址处。所以，pop 指令地址为 `0x400547 - 0x2 = 0x400545` 。**注意，ja 指令的编码需要 2 个字节。**所以 ja 指令的地址为 `0x400543`处。

```
4005e8: e9 73 ff ff ff     jmp  XXXXXX
4005ed: 90                 nop
```

> 偏移量是 `0xffffff73` ，也就是 `-141` 。所以跳转目标为 `0x4005ed - 141 = 0x400560` 

##### 练习3.18

```
long test(long x, long y, longz) {
    long val = x + y + z;
    if (x < -3) {
        if (y < z)
            val = x * y;
        else
            val = y * z;
    } else if (x > 2) 
        val = x * z;
   return val;
}
```



##### 练习3.21

```
long test(long x, long y) {
    long val = 8 * x;
    if (y > 0) {
        if (x < y)
            val = y - x;
        else
            val = x & y;
    } else if (y <= -2)
        val = x + y;
    return val;
}
```

#### 3.6.7 循环

##### 练习3.23

A.`%rax` 存放 x ，`%rcx` 存放 y ，`%rdx` 存放 n 

B. 用`leaq`指令 实现+y和++;

##### while循环

while语句的通用形式如下:

```
while(test-expr):

​	body-statement
```



1. 第一种翻译方法,我们称之为`跳转到中间`,它执行一个无条件跳转跳到循环结尾处的测试,以此来执行初始的测试.

```
	goto test;

loop:

​	body-statement

test:

​	t=test-expr;

​	if(t)

​		goto loop;
```

2. 第二种翻译方法,我们称之为`guarded-go`,首先用条件分支,如果初始条件不成立就跳过循环,把代码变换为`do-while`循环.

```
t=test-expr;

if(!t)

​	goto done;

loop:

​	body-statement;

​	t=test-expr;

​	if(t)

​		goto loop;

done;
```



##### 练习3.24

```
long loop_while(long a, long b)
{
    long result = 1;
    while (a < b) {
        result = result * (a + b);
        a = a + 1 ;
    }
    return result;
}
```

##### 练习题3.26

1. `jmp`

2. ```
   long fun_a(unsigned long x) {
       long val = 0;
       while (x) {
           val ^= x;
           x >>= 1;
       }
       return val & 0x1;
   }
   ```

3. 奇偶校验

##### for循环:

```
for(init-expr;test-expr;update-expr)

​	body-statement
```

等价于

```
init-expr;

while(test-expr){

​	body-statement;

​	update-expr

}
```

GCC为for循环产生的代码时while循环的两种翻译之一.

##### 练习3.28

A.

```
long fun_b(unsigned long x) {
    long val = 0;
    long i;
    for (i = 64; i != 0; i--) {
        val = (val * 2) | (x & 0x1);
        x >>= 1;
    }
    return val;
}
```

B.因为是使用guarded-do生成的,而初始化为0x40,一定满足,所以没必要测试.

C.reverse

##### 练习3.29

A. 导致i++无法执行,进入死循环.

B. 用goto跳过body-statement.

#### 3.6.8 switch语句

swtich 语句通过使用`跳转表`这种数据结构使得实现更加高效.\

GCC扩展了C语言, 加入了&&运算符用来取得指向标号的指针, 然后组成一个指针数组. 这样就可以用测试的值运算后得到的值直接进行索引来跳转.

` jmp *.L4(, %rsi, 8)`

跳转表:

```
.L4:
    .quad    .L3
    .quad    .L8
    .quad    .L5
    .quad    .L6
    .quad    .L7
```

##### 练习3.30

分析汇编:

```
# void switch2(long x, long *dest)
# x in %rdi
switch2:
    addq    $1, %rdi    # x = x + 1 ，所以 x 的最小值是 -1    
    cmpq    $8, %rdi    # 7结束   
    ja      .L2         # 超过 8 就跳转到 L2，所以 L2 相当于 default   
    jmp     *.L4(, %rdi, 8)    # 没有超过 8 就进入跳转表
```

##### 练习3.31

```
void switcher(long a, long b, long c, long *dest)
{
    long val;
    switch(a) {
    case 5:     /* Case A */
        c = b ^ 15;
        /* Fall through */
    case 0:     /* Case B */
        val = c + 112;
        break;
    case 2:     /* Case C */
    case 7:     /* Case D */
        val = (c + b) << 2;
        break;
    case 4:     /* Case E */
        val = a;
        break;
    default:
        val = b;
    }
    *dest = val;
}
```



#### 3.7过程

过程是软件中一种很重要的抽象.它提供了一种封装代码的方式,用一组指定的参数和一个可选的返回值实现了某种功能.过程在不同的语言中的表现形式不同, 比如函数, 方法等.

假设过程P调用过程Q:

* 传递控制: 在进入过程Q的时候,程序计数器必须被设置为Q的代码的初始地址,然后在返回时,要把程序计数器设置为P中调用Q后面那条指令的地址.
* 传递数据: P必须能向Q提供一个或多个参数,Q也能返回一个值.
* 分配和释放内存: Q可能需要为局部变量分配内存,返回时释放内存.

##### 3.7.1 运行时栈

C语言过程调用机制的一个关键特征在于使用了栈数据结构提供的后进先出的内存管理原则.

##### 3.7.3 数据传送

其实就是调用规则.

| 操作数大小 | 参数数量 |      |      |      |      |      |
| :--------: | :------: | :--: | :--: | :--: | :--: | ---- |
|            |    1     |  2   |  3   |  4   |  5   | 6    |
|     64     |   %rdi   | %rsi | %rdx | %rcx | %r8  | %r9  |
|     32     |   %edi   | %esi | %edx | %ecx | %r8d | %r9d |
|     16     |   %di    | %si  | %dx  | %cx  | %r8w | %r9w |
|     8      |   %dil   | %sil | %dl  | %cl  | %r8b | %r9b |

多于6个参数的放栈上.

##### 练习3.33

根据 size得知 sizeof(a)+sizeof(b)=6

所以一个为4,一个为2.且4扩8,2变1.

`int procprob(int a, short b, long *u, char *v)`
`int procprob(int b, short a, long *v, char *u)`

#### 3.7.5 寄存器中的局部存储空间

根据惯例,寄存器`%rbx`,`%rbp`和`%r12~%r15`被划分为`被调用者保存寄存器`.

所有其他的寄存器,除了栈指针`%rsp`,都分类为`调用者保存寄存器`



##### 练习3.34

A.局部值 a0 ~ a5 分别保存被调用者保存寄存器 %rbx、%r15、%r14、%13、%12 和 %rbp。

B.局部值 a6 和 a7 存放在栈中相对于栈指针偏移量为 0 和 8 的地方。

C. 因为用于存临时变量的寄存器只有6个. 

##### 练习3.35

寄存器保存参数x的值

```
long rfun(unsigned long x){
    if( x == 0 ){
        return return 0;
    }
    unsigned long nx = x >> 2;
    long rv = rfun(nx);
    return x + rv;
}
```

#### 3.8 数组分配和方问

#### 3.8.1 基本原则

假设E是一个int型的数组,而我们想计算E[i],在此,E的地址存放在寄存器%rdx中,而i存放在寄存器%rcx中.内存引用指令为:

>  movl (%rdx,%rcx,4),%eax

#### 3.9.1 结构

类似于数组的实现,结构的所有组成部分都存放在内存中一段连续的区域内,而指向结构的指针就是结构第一字节的地址.

rp->width等价于表达式(*rp).width.

结构的各个字段的选取完全是在编译时处理的.机器代码不包含关于字段声明或字段名字的信息.

##### 练习3.42

```
long fun(struct ELE *ptr){
    long value = 0;
    while(ptr!=NULL){
        value += ptr->v;
        ptr= ptr->p;
    }
    return value;
}
```

#### 3.9.2联合

联合的空间始终等于其中最大的元素所占据的空间. 联合的一个优点是, 以不同的数据类型去访问数据的时候, 位级表示是一样的.

如果数据结构中存在很多互斥的数据,使用联合能够节省非常大的空间.

##### 练习3.43

|         expr         | type  |                             代码                             |
| :------------------: | :---: | :----------------------------------------------------------: |
|      `up->t1.u`      | long  |            `movq (%rdi), %rax movq %rax, (%rsi)`             |
|      `up->t1.v`      | short |             `movw 8(%rdi), %ax movw %ax, (%rsi)`             |
|     `&up->t1.w`      | char* |              `addq $10, %rdi movq %rdi, (%rsi)`              |
|      `up->t2.a`      | int*  |                     `movq %rdi, (%rsi)`                      |
| `up->t2.a[up->t1.u]` |  int  | `movq (%rdi), %rax movl (%rdi, %rax, 4), %eax movl %eax, (%rsi)` |
|     `*up->t2.p`      | char  |    `movq 8(%rdi), %rax movb (%rax), %al movb %al, (%rsi)`    |

#### 3.9.3 数据对齐

对齐限制简化了形成处理器和内存系统之间接口的硬件设计.

##### 练习3.44

A. `struct P1 {int i; char c; int j; char d};`

|  i   |  c   |  j   |  d   | 总共 | 对齐 |
| :--: | :--: | :--: | :--: | :--: | :--: |
|  0   |  4   |  8   |  12  |  16  |  4   |

B. `struct P2 {int i; char c; char d; long j};`

|  i   |  c   |  j   |  d   | 总共 | 对齐 |
| :--: | :--: | :--: | :--: | :--: | :--: |
|  0   |  4   |  5   |  8   |  16  |  8   |

C. `struct P3 {short w[3]; char c[3]};`

|  w   |  c   | 总共 | 对齐 |
| :--: | :--: | :--: | :--: |
|  0   |  6   |  10  |  2   |

D. `struct P4 {short w[5]; char *c[3]};`

|  w   |  c   | 总共 | 对齐 |
| :--: | :--: | :--: | :--: |
|  0   |  16  |  40  |  8   |

E. `struct P5 {struct P3 a[2]; struct P2 t};`

|  a   |  t   | 总共 | 对齐 |
| :--: | :--: | :--: | :--: |
|  0   |  24  |  40  |  8   |

##### 练习3.45

A.

|  字段  |  a   |  b   |  c   |  d   |  e   |  f   |  g   |  h   |
| :----: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
|  大小  |  8   |  2   |  8   |  1   |  4   |  1   |  8   |  4   |
| 偏移量 |  0   |  8   |  16  |  24  |  28  |  32  |  40  |  48  |

B. 56 个字节长。

C. 从大往小排 40

#### 3.10.1 理解指针

* 每个指针都对应一个类型.这个类型表明该指针指向的是哪一类对象.
  * 指针类型不是机器代码中的一部分
  * 它们是C语言提供的一种抽象,帮助程序员避免寻址错误
* 每个指针都有一个值
  * 这个值是某个指定类型的对象的地址
  * 特殊的NULL(0)值表示该指针没有指向任何地方
* 指针用'&'运算符创建
  * leaq指令是设计用来计算内存引用的地址的.
  * &运算符的机器代码实现常常用这条指令来计算表达式的值
* *操作符用于间接引用指针.
  *  间接引用是用内存引用来实现的
* 数组与指针紧密联系
  * 数组引用(a[3])与指针运算和间接引用(*(a+3))有一样的效果
* 将指针从一种类型强制转换成另一种类型,只改变它的类型,而不改变它的值.
* 指针也可以指向函数.
  * 函数指针的值是该函数机器代码表示中第一条指令的地址.