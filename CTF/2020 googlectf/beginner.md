这道题是个简单的签到题

核心思路是:

> (flag[shuffle[i]]+add[i])^xor[i]==flag[i]

通过已有条件逐步推出所有flag的一道题

需要注意的是add运用的是`_mm_add_epi32`这个函数,所以有时会产生进位对相邻flag位造成影响



可以使用z3来解,我的做法是直接推导.

脚本如下:

```python
def cal(num):

​    a=flag[num]

​    for i in range(16):

​        if num==shuffle[i]:

​            b=i 

​            break

​    x=ord(a)+add[b]

​    if(x>256):

​        x%=256

​        if(b%4!=0):

​            add[b+1]+=1

​    flag[b]=chr(x^xor[b])

​    return 



shuffle=[0x02, 0x06, 0x07, 0x01, 0x05, 0x0B, 0x09, 0x0E, 0x03, 0x0F, 0x04, 0x08, 0x0A, 0x0C, 0x0D, 0x00]

add=[  0xEF, 0xBE, 0xAD, 0xDE, 0xAD, 0xDE, 0xE1, 0xFE, 0x37, 0x13, 0x37, 0x13, 0x66, 0x74, 0x63, 0x67]

xor=[ 0x76, 0x58, 0xB4, 0x49, 0x8D, 0x1A, 0x5F, 0x38, 0xD4, 0x23, 0xF8, 0x34, 0xEB, 0x86, 0xF9, 0xAA]



flag=[0]*16

flag[0]='C'

flag[1]='T'

flag[2]='F'

flag[3]='{'

flag[14]='}'

flag[15]=chr(0)



cal(3)

cal(8)

cal(11)

cal(5)

cal(4)

cal(10)

cal(12)

cal(15)

cal(9)

cal(14)

flag_str=""

for i in range(16):

​    flag_str+=flag[i]

print(flag_str)

```

