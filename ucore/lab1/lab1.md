### 练习1

#### tips:

如想了解make执行了哪些命令，可以执行：

```
$ make "V="
```

另外可以查看makefile的一个文档:

https://seisman.github.io/how-to-write-makefile/introduction.html

#### 任务

在此练习中，大家需要通过静态分析代码来了解：

1. 操作系统镜像文件ucore.img是如何一步一步生成的？(需要比较详细地解释Makefile中每一条相关命令和命令参数的含义，以及说明命令导致的结果)

> 用了gcc 把c代码编译成.o文件(也是目标文件)
>
> 用ld  把目标文件转换成执行程序 (链接)
>
> 用dd  把bootblock放到虚拟硬盘(可以理解成copy)

2. 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？

首先用一张图介绍一下硬盘主引导扇区:

![1.png](https://i.loli.net/2020/09/06/3R2tvKlcxFhroQE.png)

> 阅读源码lab1/tools/sign.c,可以发现硬盘主引导扇区的特征是512字节,且最后两字节是结束标志字0x55和0xAA.



### 练习2

#### 任务

为了熟悉使用qemu和gdb进行的调试工作，我们进行如下的小练习：

1. 从CPU加电后执行的第一条指令开始，单步跟踪BIOS的执行。

2. 在初始化位置0x7c00设置实地址断点,测试断点正常。

   这里借用了@Kiprey师傅的做法.

   > 将gdbinit修改为
   >
   > file obj/bootblock.o
   > set architecture i8086
   > target remote :1234
   > b* 0x7c00
   >
   > define hook-stop
   > x/i $eip
   > end
   > continue
   >
   > 通过这几行 可以展示后面的代码

3. 从0x7c00开始跟踪代码运行,将单步跟踪反汇编得到的代码与bootasm.S和 bootblock.asm进行比较。

   > 执行make debug命令. 
   >
   > 然后比对,肉眼简单感觉没什么区别.

4. 自己找一个bootloader或内核中的代码位置，设置断点并进行测试

### 练习3

#### 任务

BIOS将通过读取硬盘主引导扇区到内存，并转跳到对应内存中的位置执行bootloader。请分析bootloader是如何完成从实模式进入保护模式的。

- 为何开启A20，以及如何开启A20

  - 为了实现向后兼容
  - **A20 Gate**的作用是做到和Intel早期的8086 CPU的回卷一样的效果，0，关闭，就指明超出1MB的全被回卷，1，开启才能访问4GB的内存
  - 如何开启
    - 等待8042 Input buffer为空
    - 发送Write 8042 Output Port （P2） 命令到8042 Input buffer
    - 等待8042 Input buffer为空
    - 将8042 Output Port（P2） 对应字节的第2位置1，然后写入8042 Input buffer

  在当前环境中，所用到的键盘控制器8042的IO端口只有0x60和0x64两个端口

  seta20.1是往端口0x64写数据0xd1，往8042芯片的P2端口写数据；

  seta20.2是往端口0x60写数据0xdf，8042芯片的P2端口设置为1

  ```
  seta20.1:
      inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
      testb $0x2, %al
      jnz seta20.1
  
      movb $0xd1, %al                                 # 0xd1 -> port 0x64
      outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port
  
  seta20.2:
      inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
      testb $0x2, %al
      jnz seta20.2
  
      movb $0xdf, %al                                 # 0xdf -> port 0x60
      outb %al, $0x60                                 # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1
  ```

  

- 如何初始化GDT表

```
# Bootstrap GDT
.p2align 2                                          # force 4 byte alignment
gdt:
    SEG_NULLASM                                     # null seg
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel

gdtdesc:
    .word 0x17                                      # sizeof(gdt) - 1
    .long gdt                                       # address gdt
```

* 
    * 设置GDT中的第一项描述符为null
    * 设置GDT中的第二项描述符为代码段使用



- 如何使能和进入保护模式

```
    ***进入保护模式***
	#通过将cr0寄存器PE置1
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0
    #长跳转更新cs的基地址
    ljmp $PROT_MODE_CSEG, $protcseg
```



简略看一下完成的汇编

```
#include <asm.h>

# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG,        0x8                     # kernel code segment selector
.set PROT_MODE_DSEG,        0x10                    # kernel data segment selector
.set CR0_PE_ON,             0x1                     # protected mode enable flag

# start address should be 0:7c00, in real mode, the beginning address of the running bootloader
***初始化***
.globl start
start:
.code16                                             # Assemble for 16-bit mode
    cli                                             # Disable interrupts
    cld                                             # String operations increment

    # Set up the important data segment registers (DS, ES, SS).
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment
    movw %ax, %ss                                   # -> Stack Segment

***开启A20***
    # Enable A20:
    #  For backwards compatibility with the earliest PCs, physical
    #  address line 20 is tied low, so that addresses higher than
    #  1MB wrap around to zero by default. This code undoes this.
seta20.1:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.1

    movb $0xd1, %al                                 # 0xd1 -> port 0x64
    outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port

seta20.2:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.2

    movb $0xdf, %al                                 # 0xdf -> port 0x60
    outb %al, $0x60                                 # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1

    # Switch from real to protected mode, using a bootstrap GDT
    # and segment translation that makes virtual addresses
    # identical to physical addresses, so that the
    # effective memory map does not change during the switch.
    ***初始化GDT表***
    lgdt gdtdesc
    ***进入保护模式***
#通过将cr0寄存器PE位置1便开启了保护模式
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    #长跳转更新cs的基地址
    ljmp $PROT_MODE_CSEG, $protcseg

.code32                                             # Assemble for 32-bit mode
protcseg:
	# 设置段寄存器,并建立堆栈
    # Set up the protected-mode data segment registers
    movw $PROT_MODE_DSEG, %ax                       # Our data segment selector
    movw %ax, %ds                                   # -> DS: Data Segment
    movw %ax, %es                                   # -> ES: Extra Segment
    movw %ax, %fs                                   # -> FS
    movw %ax, %gs                                   # -> GS
    movw %ax, %ss                                   # -> SS: Stack Segment

    # Set up the stack pointer and call into C. The stack region is from 0--start(0x7c00)
    movl $0x0, %ebp
    movl $start, %esp
    #保护模式完成
    call bootmain

    # If bootmain returns (it shouldn't), loop.
spin:
    jmp spin

# Bootstrap GDT
.p2align 2                                          # force 4 byte alignment
gdt:
    SEG_NULLASM                                     # null seg
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel

gdtdesc:
    .word 0x17                                      # sizeof(gdt) - 1
    .long gdt                                       # address gdt


```

### 练习4

#### 任务

通过阅读bootmain.c，了解bootloader如何加载ELF文件。通过分析源代码和通过qemu来运行并调试bootloader&OS，

- bootloader如何读取硬盘扇区的？
  - 等待磁盘就绪
  - 设置磁盘参数
  - 等待磁盘就绪
  - 读取数据

可以看如下代码:

```
static void
readsect(void *dst, uint32_t secno) {
    // wait for disk to be ready
    waitdisk();

    outb(0x1F2, 1);                         // count = 1 读一个扇区
    outb(0x1F3, secno & 0xFF);              // LBA模式 0-7 8-15 16-23
    outb(0x1F4, (secno >> 8) & 0xFF);
    outb(0x1F5, (secno >> 16) & 0xFF);
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);
    outb(0x1F7, 0x20);                      // cmd 0x20 - read sectors

    // wait for disk to be ready
    waitdisk();

    // read a sector
    insl(0x1F0, dst, SECTSIZE / 4);         //读取到dst
}
```



- bootloader是如何加载ELF格式的OS？



可以看如下代码:

```
   //首先读取ELF的头部
    readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);

    //判断
    if (ELFHDR->e_magic != ELF_MAGIC) {
        goto bad;
    }

    struct proghdr *ph, *eph;

    //加载表在ELF文件结构中的偏移,获取程序表的数量.
    ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
    eph = ph + ELFHDR->e_phnum;
    //加载入内存
    for (; ph < eph; ph ++) {
        readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
    }
	//寻找内核入口
    // call the entry point from the ELF header
    // note: does not return
    ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();
```

### 练习5

#### 任务

我们需要在lab1中完成kdebug.c中函数print_stackframe的实现，可以通过函数print_stackframe来跟踪函数调用堆栈中记录的返回地址。

文件在lab1/kern/debug/kdebug.c

核心思想是`caller's ebp = *(callee's ebp)`

```
void print_stackframe(void) {
    uint32_t ebp = read_ebp();
    uint32_t eip = read_eip();
    for(uint32_t i = 0; ebp != 0 && i < STACKFRAME_DEPTH; i++)
    {
        cprintf("ebp:0x%08x eip:0x%08x args:", ebp, eip);
        uint32_t* args = (uint32_t*)ebp + 2 ;
        for(uint32_t j = 0; j < 4; j++)
            cprintf("0x%08x ", args[j]);
        cprintf("\n");
        // call print_debuginfo(eip-1) to print the C calling function name and line number
        print_debuginfo(eip-1);
        //  先设置eip后设置ebp
        eip = *((uint32_t*)ebp + 1);
        ebp = *(uint32_t*)ebp;
    }
}
```

### 练习6

#### 任务:完善中断初始化和处理 

请完成编码工作和回答如下问题：

1. 中断描述符表（也可简称为保护模式下的中断向量表）中一个表项占多少字节？其中哪几位代表中断处理代码的入口？

   表结构如下:

   ```
   /* Gate descriptors for interrupts and traps */
   struct gatedesc {
       unsigned gd_off_15_0 : 16;        // low 16 bits of offset in segment
       unsigned gd_ss : 16;            // segment selector
       unsigned gd_args : 5;            // # args, 0 for interrupt/trap gates
       unsigned gd_rsv1 : 3;            // reserved(should be zero I guess)
       unsigned gd_type : 4;            // type(STS_{TG,IG32,TG32})
       unsigned gd_s : 1;                // must be 0 (system)
       unsigned gd_dpl : 2;            // descriptor(meaning new) privilege level
       unsigned gd_p : 1;                // Present
       unsigned gd_off_31_16 : 16;        // high bits of offset in segment
   };
   ```

   > 占64个bit,共8个字节.
   >
   > gd_ss为IDT表项的选择子,gd_off_15_0和gd_off_31_16共同组成offset偏移,两者组合可以代表中断处理代码的入口.

2. 请编程完善kern/trap/trap.c中对中断向量表进行初始化的函数idt_init。在idt_init函数中，依次对所有中断入口进行初始化。使用mmu.h中的SETGATE宏，填充idt数组内容。每个中断的入口由tools/vectors.c生成，使用trap.c中声明的vectors数组即可。

```
void idt_init(void) {
  extern uintptr_t __vectors[];
  int i;
  for (i = 0; i < sizeof(idt) / sizeof(struct gatedesc); i ++)
      // 目标idt项为idt[i]
      // 0代表中断门
      // GD_KTEXT为代码段
      // 中断处理程序的入口地址存放于__vectors[i]
      // 特权级为DPL_KERNEL
      SETGATE(idt[i], 0, GD_KTEXT, __vectors[i], DPL_KERNEL);
  //用户段切换到内核态
  SETGATE(idt[T_SWITCH_TOK], 0, GD_KTEXT, __vectors[T_SWITCH_TOK], DPL_USER);
  // 加载
  lidt(&idt_pd);
}
```

3. 请编程完善trap.c中的中断处理函数trap，在对时钟中断进行处理的部分填写trap函数中处理时钟中断的部分，使操作系统每遇到100次时钟中断后，调用print_ticks子程序，向屏幕上打印一行文字”100 ticks”。

```
/* trap_dispatch - dispatch based on what type of trap occurred */
static void trap_dispatch(struct trapframe *tf) {
    char c;
    switch (tf->tf_trapno) {
    case IRQ_OFFSET + IRQ_TIMER:
        ticks++;
        if(ticks % TICK_NUM == 0)
            print_ticks();
        break;
```

注意: 全局变量ticks定义于kern/driver/clock.c

### 拓展:

涉及到特权级切换 所以写完lab2再回来补充.