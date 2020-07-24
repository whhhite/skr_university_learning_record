## datalab

做的2019更新的新版本.

这个lab主要考察的是用位运算来写函数.



#### bitXor

题目要求: 是仅使用`~`和`&`来实现.

首先让我们列一下异或的真值表:

|  A   |  B   | A^B  |
| :--: | :--: | :--: |
|  0   |  0   |  0   |
|  0   |  1   |  1   |
|  1   |  0   |  1   |
|  1   |  1   |  0   |

我们可以用 `A&B`和`(~A&~B)`代替第四行和第一行

所以可以使用`A&B`和`(~A&~B)`取反在求交集得到异或.

代码如下:

```
int bitXor(int x, int y) {
	return ~(x & y) & ~(~x & ~y);
}
```



#### tmin

题目要求: 最小的int类型的补码即Tmin32;

> Tmin32=0x80000000=-2147483648

所以只要1<<31就可以得到.

代码如下:

```
int tmin(void) {
	return 1<<31;
}
```



#### isTmax

题目要求: 判断x是不是Tmax;

> Tmax32=0x7FFFFFFF;

Tmax32有什么特点? +1溢出到0x8000000.

如果两者异或的话可以得到-1

再排除掉-1的情况即可.

代码如下:

```
int isTmax(int x) {
	return !(~(x ^ (x + 1))) & !!(~x);
}
```



另一个方法是:

彻底溢出 即 2*(Tmax32+1)=0;

这种情况则要排除0xFFFFFFFF(也是-1)的情况.



#### allOddBits

题目要求: 判断是否所有奇数位为1

> 首先取掩码0x55555555,0x55555555是所有偶数位为1

这样把x和掩码进行`|`运算,可以得到0xFFFFFFFF.

再通过`!~`即可.

代码如下:

```
int allOddBits(int x) {
	return !~(x|0x55555555);
}
```



#### negate

题目要求: 求相反数

取反+1即可.

代码如下:

```
int negate(int x) {
	return ~x+1;
}
```



#### isAsciiDigit

题目要求: 判断x是否是自然数'0'~'9'

> 0x30 = '0'  = 0x110000 
>
> 0x39 = '9'  = 0x111001

可以通过两个条件确定范围:

> x-0x30>=0
>
> x-0x3a<0

```
int isAsciiDigit(int x) {
	return(!((x+~0x30+1)>>31))&!!((x+~0x3a+1)>>31);
}
```



#### conditonal

题目要求: 实现x ? y : z

> x=1 输出 y
>
> x=0 输出 z

可以采用0xFFFFFFFF和0x0做掩码 来获得不同的值.

当x=1时:

(0xFFFFFFFF&y)| (0x0&z)

当x=0时:

(0x0&y)| (0xFFFFFFFF&z)

左边可以采用: !x+(-1)的形式

右边可以采用: ~!x+1的形式

代码如下:

```
int conditional(int x, int y, int z) {
	return ((!x+~1+1)&y)|((~!x+1)&z);
}
```



#### isLessOrEqual

题目要求: 实现 x<=y 即 y-x>=0

1. 符号位相同 y+(-x) 再判断符号
2. 符号位不同, 则只能y为正

代码如下:

```
int isLessOrEqual(int x, int y) {
 	//异号:
	int sign_x = x>>31;
	int sign_y = y>>31;
	int diff = (sign_x^sign_y)&(!sign_y);
	//同号:
	int sign_delta = (y+~x+1)>>31;
	int same = !(sign_x^sign_y)&(!sign_delta);
	return diff | same;
}
```



#### logicalNeg

题目要求: 实现 `!`

其实就是判断是否为0.

采用取相反数判断符号位的方法:

一共三种情况:

1. 为0时, x 与 -(x) 符号位都为0
2. 为Tmin时, x与 -(x) 符号位都为 -1
3. 其他情况不同.

只要提取出第一种情况即可.

```
int logicalNeg(int x) {
    return ((x >> 31) | ((~x + 1) >> 31)) + 1;
}
```



#### howManyBits

题目要求: 判断有多少位.

二分法写法.

首先判断取的段是否为0.再移位.

从16一直到1位.

代码如下:

```cpp

```

浮点数这部分基础比较差,都是参考网上的做法的.

#### floatScale2:

题目要求: 计算2*f

代码如下:

```
unsigned floatScale2(unsigned uf) {
    //浮点数float：符号位1位，指数8位，数据位23位
    //0[11111111]0..0 作为指数部分的掩码
    int exp_mask = 0x7f800000;
    //1[00000000]1..1 作为除指数之外部分的掩码
    int anti_exp_maks = 0x807fffff;
    //exp 表示指数
    int exp = ((uf & exp_mask) >> 23);
    //sign 表示符号位
    int sign = uf & 0x80000000;
    //非规格化数：直接左移一位
    //  临界情况：非规格化数左移为规格化数：仔细理解定义
    //  符号位需要重置一下
    if(exp == 0) return (uf << 1) | sign;
    //无穷大或 NaN ：直接返回
    if(exp == 255) return uf;
    //规格化数：指数部分加 1 ，再做溢出判定
    exp ++;
    if(exp == 255) return exp_mask | sign;
    return (exp << 23) | (uf & anti_exp_maks);
}
```



#### floatFloat2Int

题目要求: float转int.

代码如下:

```
int floatFloat2Int(unsigned uf) {
    //基本思路：将 23 位的 frac 根据 exp 的数值做小数点的浮动，再将整数部分写入 32 位的 int 型
    int sign = uf >> 31;
    int exp = ((uf & 0x7f800000) >> 23) - 127;
    //截取 frac 的 23 位，并将第 24 位数置 1（根据浮点数定义，规格化数需要补 1）
    int frac = (uf & 0x007fffff) |  0x00800000;
    //uf 为 0
    if(!!(uf & 0x7fffffff)) return 0;
    //做溢出处理
    if(exp > 31) return 0x80000000;
    //uf 为非规格化数（整数部分为 0）
    if(exp < 0) return 0;
    //根据浮点数定义，截取整数部分
    if(exp > 23) frac <<= (exp - 23);
    else frac >>= (23 - exp);
    //如果符号位相同，直接返回
    if(!((frac >> 31) ^ sign)) return frac;
    //如果 uf 符号位为 0，得到的结果符号位为 1 ：溢出
    else if(frac >> 31) return 0x80000000;
    //如果 uf 符号位为 1，得到的结果符号位为 0 ：取反
    else return ~frac + 1;
}
```



#### floatPower2

题目要求: 实现2.0^x

代码如下:

```
unsigned floatPower2(int x) {
    //2.0^x = 1.0 * 2^x
    //根据定义，计算 exp 的数值
    int exp = x + 127;
    //判 0 处理
    if(exp <= 0) return 0;
    //溢出处理
    if(exp >= 255) return 0x7f800000;
    return exp << 23;
}
```

