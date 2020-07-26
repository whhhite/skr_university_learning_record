## archlab 

### 环境配置

看起来gui不是必要的,所以直接按照别人的方法注释了

```
#GUIMODE=-DHAS_GUI
#TKLIBS=-L/usr/lib -ltk -ltcl
#TKINC=-isystem /usr/include
```

然后就可以make了.

### part A

文件在sim/misc里

提供了examples.c代码.要翻译成Y86-64架构下的汇编.

```
* Example programs for Part A of the CS:APP Architecture Lab
examples.c		C versions of three Y86 functions 
ans-copy.ys		Solution copy function (instructor distribution only)
ans-sum.ys		Solution sum function (instructor distribution only)
ans-rsum.ys		Solution rsum function (instructor distribution only)
```

根据REAMME.md,可以看出要提交3个ys.

参考了一点书上的代码(P252)和网上的代码.

#### sum_list

实现求和

需要实现的c代码:

```
typedef struct ELE {
    long val;
    struct ELE *next;
} *list_ptr;

/* sum_list - Sum the elements of a linked list */
long sum_list(list_ptr ls)
{
    long val = 0;
    while (ls) {
	val += ls->val;
	ls = ls->next;
    }
    return val;
}
```

ans-sum.ys

```
# Execution begins at address 0
		.pos 0
		irmovq stack , %rsp
		call main
		halt
//list结构 以及数据是0x00a,0x0b0,0xc00(为什么是这个呢,问就是偷的.)
		.align 8
 	ele1:
		.quad 0x00a
		.quad ele2
  	ele2:
		.quad 0x0b0
		.quad ele3
  	ele3:
		.quad 0xc00
		.quad 0
//main函数
	main:
		irmovq ele1,%rdi
		call sumlist
		ret

//sumlist
	sumlist:
		xorq %rax,%rax     //long val=0
		andq %rdi,%rdi     //while (ls)
		je end		       //ret
		irmovq $8 , %r8    //设置常数8
	loop:
		mrmovq (%rdi),%rcx //ls->val
		addq %rcx,%rax     //val += ls->val
		addq %r8,%rdi      //ls->next;
		mrmovq (%rdi),%rdi //ls=ls->next
		andq %rdi,%rdi     //while (ls)
		jne loop
	end:
		ret                //return val
# Stack starts here and grows to lower addresses
.pos 0x200
stack: 
```

结果:

```
Stopped in 29 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:	0x0000000000000000	0x0000000000000cba
%rcx:	0x0000000000000000	0x0000000000000c00
%rsp:	0x0000000000000000	0x0000000000000200
%r8:	0x0000000000000000	0x0000000000000008

Changes to memory:
0x01f0:	0x0000000000000000	0x000000000000005b
0x01f8:	0x0000000000000000	0x0000000000000013
```

%rax是sum_list最后返回的和.

#### rsum_list

递归调用版本

需要实现的c代码:

```
long rsum_list(list_ptr ls)
{
    if (!ls)
	return 0;
    else {
	long val = ls->val;
	long rest = rsum_list(ls->next);
	return val + rest;
    }
}
```

ans-rsum.ys

```
# Execution begins at address 0
		.pos 0
		irmovq stack , %rsp
		call main
		halt
//list结构
		.align 8
 	ele1:
		.quad 0x00a
		.quad ele2
  	ele2:
		.quad 0x0b0
		.quad ele3
  	ele3:
		.quad 0xc00
		.quad 0
//main函数
	main:
		irmovq ele1,%rdi
		xorq %rax,%rax			//val清空
		call rsumlist
		ret

//rsumlist
	rsumlist:
		pushq %rcx    			//存val
		andq %rdi,%rdi    	    //if (!ls)
		je end					//ret
		mrmovq (%rdi),%rcx 	    //ls->val  
		irmovq $8 , %r8         //设置常数8
		addq %r8,%rdi      		//ls->next
		mrmovq (%rdi),%rdi		//ls=ls->next
		call rsumlist           //rsum_list(ls->next)
		addq %rcx,%rax     		//return val + rest
	end:
		popq %rcx
		ret       
# Stack starts here and grows to lower addresses
.pos 0x200
stack: 
```

运行结果:

```
Stopped in 45 steps at PC = 0x13.  Status 'HLT', CC Z=0 S=0 O=0
Changes to registers:
%rax:	0x0000000000000000	0x0000000000000cba
%rsp:	0x0000000000000000	0x0000000000000200
%r8:	0x0000000000000000	0x0000000000000008

Changes to memory:
0x01b8:	0x0000000000000000	0x0000000000000c00
0x01c0:	0x0000000000000000	0x0000000000000094
0x01c8:	0x0000000000000000	0x00000000000000b0
0x01d0:	0x0000000000000000	0x0000000000000094
0x01d8:	0x0000000000000000	0x000000000000000a
0x01e0:	0x0000000000000000	0x0000000000000094
0x01f0:	0x0000000000000000	0x000000000000005d
0x01f8:	0x0000000000000000	0x0000000000000013
```

#### copy_block

实现数组的复制,并返回数量.

需要实现的c代码:

```
long copy_block(long *src, long *dest, long len)
{
    long result = 0;
    while (len > 0) {
	long val = *src++;
	*dest++ = val;
	result ^= val;
	len--;
    }
    return result;
}
```

ans-copy.ys

```
# Execution begins at address 0
		.pos 0
		irmovq stack , %rsp
		call main
		halt

		.align 8
	src:
		.quad 0x000a
		.quad 0x00b0
		.quad 0x0c00
		.quad 0xd000
	dest:
		.quad 0x1111
		.quad 0x2222
		.quad 0x3333
		.quad 0x4444
	main:
		irmovq src,%rdi			//long *src
		irmovq dest,%rsi		//long *dest
		irmovq $4,%rdx			//long len
		call copyblock
		ret
	copyblock:
		xorq %rax,%rax			//long result = 0;
	loop:
		andq %rdx,%rdx			//while (len > 0)
		jle end
		mrmovq (%rdi),%rcx		//val = *src
		irmovq  $8 , %r8		//存常数8
        addq    %r8 , %rdi		//*src++
        rmmovq  %rcx , (%rsi)	//*dest = val
        addq    %r8 , %rsi		//*dest++
        xorq    %rcx , %rax		//result ^= val
        irmovq  $1 , %rbx		//存常数1
        subq    %rbx , %rdx		//len--
        jmp     loop 
end:
        ret
# Stack starts here and grows to lower addresses
.pos 0x200
stack:
```

结果:

```
Stopped in 56 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:	0x0000000000000000	0x000000000000dcba
%rcx:	0x0000000000000000	0x000000000000d000
%rbx:	0x0000000000000000	0x0000000000000001
%rsp:	0x0000000000000000	0x0000000000000200
%rsi:	0x0000000000000000	0x0000000000000058
%rdi:	0x0000000000000000	0x0000000000000038
%r8:	0x0000000000000000	0x0000000000000008

Changes to memory:
0x0038:	0x0000000000001111	0x000000000000000a
0x0040:	0x0000000000002222	0x00000000000000b0
0x0048:	0x0000000000003333	0x0000000000000c00
0x0050:	0x0000000000004444	0x000000000000d000
0x01f0:	0x0000000000000000	0x000000000000007f
0x01f8:	0x0000000000000000	0x0000000000000013

```

### part B

在sim/seq文件夹下seq-full.hcl.

要实现iaddq.(像练习题4.51,4.52)

#### 环境配置:

记得修改makefile里面的make VERSION=full

然后三条指令测试:

```
./ssim -t ../y86-code/asumi.yo
(cd ../y86-code; make testssim)
(cd ../ptest; make SIM=../seq/ssim)
```

成功会显示Checks Succeed

#### 实验

可以参考课本P266 图4-18 来实现.

|  阶段  |                         iaddq(V,rB)                          |
| :----: | :----------------------------------------------------------: |
|  取指  | icode:ifun <-- M1[PC]<br />rA: rB <-- M1[PC+1] <br />valC <-- M8[PC+2]<br />valP <-- PC+10 |
|  译码  |                        valB <-- R[rB]                        |
|  执行  |                      valE <-- valB+valC                      |
|  访存  |                                                              |
|  写回  |                        R[rB] <-- valE                        |
| 更新PC |                         PC <-- valP                          |

然后去修改seq-full.hcl

```
//是否是合法的指令
bool instr_valid = icode in 
	{ INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
	       IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ,IIADDQ };
//是否有立即数
bool need_regids =
	icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, 
		     IIRMOVQ, IRMMOVQ, IMRMOVQ,IIADDQ };
//是否需要寄存器
bool need_valC =
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL,IADDQs };
//译码
word srcB = [
	icode in { IOPQ, IRMMOVQ, IMRMOVQ , IIADDQ  } : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't need register
];
//写回
word dstE = [
	icode in { IRRMOVQ } && Cnd : rB;
	icode in { IIRMOVQ, IOPQ, IIADDQ} : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't write any register
];
//操作数A valC是常数
word aluA = [
	icode in { IRRMOVQ, IOPQ } : valA;
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ ,IIADDQ} : valC;
	icode in { ICALL, IPUSHQ } : -8;
	icode in { IRET, IPOPQ } : 8;
	# Other instructions don't need ALU
];
//操作数B 寄存器是valB
word aluB = [
	icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, 
		      IPUSHQ, IRET, IPOPQ ,IIADDQ} : valB;
	icode in { IRRMOVQ, IIRMOVQ } : 0;
	# Other instructions don't need ALU
];
//是否需要设置条件码 OPq会影响条件码
bool set_cc = icode in { IOPQ,IIADDQ };
//PC更新没做要求
```

### part C

在sim/pipe文件夹下pipe-full.hcl实现partB

在ncopy.ys优化part A 的copy_block.

修改前:

```
CPI: 897 cycles/765 instructions = 1.17

CPE:  Average CPE	15.18
			Score	0.0/60.0
```

修改:

先实现partB

然后优化ncopy.ys

这里优化参考了网上的一篇文章的写法.

https://blog.csdn.net/u012336567/article/details/51867766

不过他是第二版的,我们使用的是第三版

优化内容是

`加载/使用数据冒险`和`循环展开`

加载/使用数据冒险是在caspp的303页.

主要就是把

```
mrmovl  (%rdi), %r8   # read val from src
rmmovl %r8, (%rsi)   # store src[0] to dest[0]
```

修改成两个一组的

```
mrmovq (%rdi),%r8 # get val from src
mrmovq 8(%rdi),%r9
rmmovq %r8,(%rsi)  #src->dest
```

然后r9再用于下一组.

循环展开比较简单,就是把迭代索引变成n.

我最后的结果是

```
68/68 pass correctness test
```

和

```
	ncopy
0	7
1	7	7.00
2	7	3.50
3	7	2.33
4	7	1.75
5	7	1.40
6	7	1.17
7	7	1.00
8	7	0.88
9	7	0.78
10	7	0.70
11	7	0.64
12	7	0.58
13	7	0.54
14	7	0.50
15	7	0.47
16	7	0.44
17	7	0.41
18	7	0.39
19	7	0.37
20	7	0.35
21	7	0.33
22	7	0.32
23	7	0.30
24	7	0.29
25	7	0.28
26	7	0.27
27	7	0.26
28	7	0.25
29	7	0.24
30	7	0.23
31	7	0.23
32	7	0.22
33	7	0.21
34	7	0.21
35	7	0.20
36	7	0.19
37	7	0.19
38	7	0.18
39	7	0.18
40	7	0.17
41	7	0.17
42	7	0.17
43	7	0.16
44	7	0.16
45	7	0.16
46	7	0.15
47	7	0.15
48	7	0.15
49	7	0.14
50	7	0.14
51	7	0.14
52	7	0.13
53	7	0.13
54	7	0.13
55	7	0.13
56	7	0.12
57	7	0.12
58	7	0.12
59	7	0.12
60	7	0.12
61	7	0.11
62	7	0.11
63	7	0.11
64	7	0.11
Average CPE	0.52
Score	60.0/60.0
```

不过怎么感觉怪怪的...

这个永远都是7 是不是测试程序有什么bug.

不过思路应该是对的.