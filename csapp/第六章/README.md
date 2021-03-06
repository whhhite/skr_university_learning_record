## 第六章 存储器层次结构

### 6.3.1

高速缓存`cache`是一个小而快速的存储设备,它作为存储在更大、也更慢的设备中的数据对象的缓冲区域.

数据总是以块大小为`传送单元`在第k层和第k+1层之间来回复制的.

* 缓存命中.
  * 程序需要第k+1层的某个数据对象d
  * d刚好缓存在第k层中
* 缓存不命中
  * 第k层没有d
  * 覆盖一个现存的块的过程称为替换或驱逐(evicting)
* 一个空的缓存有时被称为冷缓存
  * 此类不命中称为强制不命中或冷不命中
  * 冷不命中通常是短暂的事件

### 6.4 高速缓存存储器

考虑一个计算机系统,其中每个存储器地址有m位,形成M=2^m个不同的地址.

这样一个机器的高速缓存被组织成一个有S=2^s个高速缓存组的数组.

每个组包含E个高速缓存行.

每个行是由一个B=2^b字节的数据块组成的.

一个有效位指明这个是否包含有意义的信息,还有t=m-(b+s)个标记位

他们唯一地标识存储在这个高速缓存行中块.

**高速缓存的结构可以用元组(S,E,B,m)来描述**

