### Introduction 

意思是 Excel有91个需要运行的DLL, 而Notepad和calc只有30~40个.所以Excel漏洞很正常.

攻击方式是提供一个设计过的文档,让被攻击者打开.

总的来说是分析CVE-2019-0618的一个漏洞分析



### WINDOWS GDI+

GDI是一个基于C/C++ 类的API,允许应用程序在视频显示和打印机上使用图形和格式化文本. 基于Microsoft Win32和Win64  API的应用程序不直接访问图形硬件,而是使用GDI+来代表应用程序与设备驱动程序进行交互.

位于DoRotatedStretchBIt方法.



### Patch DIFFING

将BinDiff IDA插件用于修补和未修补的库.

证实了*DoRotatedStretchBlt*方法存在差异.而且在另外一个地方进行了修补,即*bHandleStretchBlt*方法.

要利用此漏洞，我们必须控制*DoRotatedStretchBlt*方法。

要用一种方法达到它,也就是IDA的call graph功能.

![img](https://www.ixiacom.com/sites/default/files/inline-images/gdi8.2.png)

可以发现*DoRotatedStretchBlt*通过调用不同的处理程序(如bHandlePlgBlt或者bHandleSetDIBitsToDev)进行操作.

这些似乎是不同EMF记录的处理程序.



### 理解EMF格式

增强型图文格式(EMF)是一种文件格式,用于存储可移植的图形图像处理表示.EMF元文件包括顺序记录,这些记录被解析和处理来适配任何设备.这些记录类型必须以某种形式在库中计算,通过这样,而在处理EMF文件时,将为其提供正确的handler. 来看*_pdofnDrawingOrders*函数.

*pdofnDrawingOrders*包括了每个handler的引用.

我们可以看到,这里枚举了所有支持的记录,包括bHandleStretchBlt这个handler.

然后去寻找DrawinOrder操作的方法.

![1594405232476](C:\Users\59634\AppData\Roaming\Typora\typora-user-images\1594405232476.png)

它这里找到 bParseWin32Metafile方法.

这个方法parse了一个WMF头.如果它包括了有效的EMF records, 它将调用正确的handler来依次解析他们.这个解析方法依次由*GdipConvertEmfToWmFBits*来调用.这里没用进一步的交叉引用,说明还有一些外部的API.通过查看Win GDI+文档.

在浏览Win GDI+文档时，GdipConvertEmfToWmFBits似乎被废弃了，但是我们在Metafile类中为它设置了一个包装器，即Metafile：：EmfToWmfBits。



现在思考一下我们干了什么:

* 易于攻击的*gdiplus!DoRotatedStretchBlt*被*gdiplus!DoStretchBlt* 调用,且这个函数handles

EMF_STRETCHBLT record type.

* 如果一个特定的flag被标记了,将会调用这个方法.
* 我们可以调用这个易于攻击的方法通过trigger了*Metafile::EmfToWmfBits*操作.其参数之一是包含EMF_STRETCHBLT记录的EMF文件。



### FUZZING 的考虑

##### 选择工具

WInAFL+Peach Fuzzing Framework

还用DynamoRio用来检测覆盖率.

##### 写一个test harness

写一个Windows GUI 应用调用了GdiPlus.dll接口.

最后看起来是这样子的.

![11](https://www.ixiacom.com/sites/default/files/inline-images/gdi11.png)

我门读一个EMF文件来作为参数.

然后初始化136行的EmfToWmfBits,希望以此触发漏洞.

然后还要释放使用过的组件和关闭打开的文件来清理环境.

#### 生成样本.

为了生成包含EMR_STRETCHBLT记录的EMF文件，我们研究了WinGDI和GDI+文档，并提出了一个简单的生成器，如下所示：

![12](https://www.ixiacom.com/sites/default/files/inline-images/gdi12.png)

这是一个包含了EMR_STRETCHBLT记录的EMF文件的生成器

我们还包括在最初的EMF文件与其他记录类型，以增加覆盖面，并有机会找到更多的崩溃。

#### 