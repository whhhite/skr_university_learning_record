### 50 CVEs in 50 Days: Fuzzing Adobe Reader

December 12, 2018

**Research By:** Yoav Alon, Netanel Ben-Simon

**Introduction**

2017年是一个漏洞行业的转折点.这一年报告的新漏洞数量大约为14000个,是前一年的两倍.造成这种情况的可能原因是自动化查找漏洞的工具(也称为"Fuzzers")越来越流行.

Fuzzers的存在并不是什么大新闻,他们已经存在了20多年了.只不过新闻是fuzzers成长了.他们变得更有能力,更容易使用,总体上更成熟.尽管如此,使用fuzzers在某种程度上还是一种"黑魔法",许多研究人员不去理会它,因为它们被认为是一种难以使用的东西.

鉴于以上所述,我们很自然的会问:是的,更多研究人员都正在使用fuzzers来发现更多的漏洞--但是,是否所有的研究人员都在使用fuzzers来查找所有漏洞呢.还有多少挂在低处的水果(指容易发现的漏洞)在那里,等着第一个人按下fuzz的闪亮大按钮呢.

为了找出答案,我们构建了我们能想到最简单的实验.我们采用了最常见的Windows模糊框架之一WinAFL,并针对AdobeReader,这是世界上最流行的软件之一.我们为整个流程设定了50天的时间框架--逆向工程他的代码,寻找潜在的易受攻击的库,编写harnesses,并最终运行Fuzzer本身.

结果让我们大吃一惊.我们阐述了整个研究的故事.我们分享了一种新的方法, 我们使用它来扩大搜索的范围,我们能够对WinAFL进行改进,最后,我们在这个过程中获得了见识.

#### What is WinAFL?

AFL是一个coverage(覆盖)引导的genetic fuzzer(带有遗传性质).它有一个可靠的实现和聪明的启发式,已经被证明了非常!成功的在真实软件中找到真正 的bugs.

WinAFL是用于Windows的AFL的一个分支,由Ivan Fratric (Google Project Zero)创建和维护.Windows版本使用了一个不同的检测方式,使我们能够锁定关闭的源二进制文件.

我们建议阅读AFL指导手册,它详细的介绍了AFL的工作原理.它还指出了该工具的缺点,并帮助您在出错时继续调试.

我们发现WinAFL在查找文件格式错误方面非常有效,特别是压缩的二进制格式(图像/视频/存档中).

#### **Attacking Acrobat Reader DC**

最容易开始的地方是主要的可执行文件,AcroRD32.exe. 这是一个(相对)薄弱的关于AcroRd32.dll的包装,它的大小大约为30MB. AcroRD32.dll有很多代码,其中包含了PDF对象的parser,但是它很多代码都是GUI代码(并不是你需要查找bugs的地方)

我们知道WinAFL在二进制格式方面更好,所以我们决定集中精力攻击特定的parser.现在的挑战是找到一个parser并为它编写一个harness.稍后我们会解释是什么是harness.

我们需要一个二进制格式parser,它具有最小的依赖关系,让我们可以在不加载整个Reader过程的情况中加载他.

我们研究了Acrobat的文件夹中的DLL,发现JP2KLib.dll适用于所有类别:

![fig2-1.png](https://i.loli.net/2020/07/05/nSYwQiCGosRfglZ.png)

JP2KLib.dll是JPEG 2000格式的解析器,它是一种复杂的二进制格式(753 KB),并导出了非常具有描述性的函数.

![fig3.jpg](https://i.loli.net/2020/07/05/XIKdflb9sSG4tYN.jpg)

对下列版本进行了研究：
Acrobat阅读器DC 2018.011.20038及更早版本
JP2KLib.dll版本1.2.2.39492

#### **What Is a Target Function?**

一个目标函数是WinAFL用来描述用作fuzzing process的入口点函数的术语.函数在循环中调用fuzz_iterations(迭代) 多次, 每次在硬盘上变异输入的文件. 这个函数必须:

* 打开输入文件, 读入文件, parse 输入 并且关闭文件
* 返回正常 -没有抛出c++错误或者调用终止函数

在自然的情况下,找到这样一个函数是不常见的.当我们瞄准一个复杂软件的时候,我们通常需要写一个harness.

#### **What Is a Harness?**

一个harness是一个小程序,它能触发我们想要的功能.该harness包括一个功能,将用于我们的目标功能.下面是一个例子是最简的harness对于WinAFL存储库的gdiplus(?):

![fig4-1.png](https://i.loli.net/2020/07/05/X3JfCg6S91wAMdN.png)

main的第一个argument是路径path. 在函数中,我门调用了 Image::Image parser,这是我们想要fuzz的API. 注意,在出现错误的情况下,我们不会终止进程,并且在结束时释放所有资源.

对于文档化的API来说,这个过程相对简单.我们可以使用文档复制一个示例代码或者编写一个简单的程序.但这其中的乐趣何在?

我们选择的目标是Adobe Reader,这是一个封闭的二进制文件.为这类目标编写harness的过程如下所示:

1. 找到我们想要fuzz的功能/函数
2. 逆向工程亿点点
3. 编写一个调用逆向过的API的程序
4. 重复直到我们有一个功能齐全的harness

在下一节,我们详细描述了如何逆向JP2KLib,并为其编写一个harness. 我们还分享了一些建议. 只对我们fuzz方法感兴趣的读者可以跳过下一个节目.

#### **Writing a Harness for JP2KLib.dll**

在开始逆向JP2KLib.dll之前,我们检查了库是不是有开源的或者有符号表.这是可以节省很多时间,并且比你想象的更正常(更好逆向的意思吧).但就我们而言,我们没这么好运.

由于我们希望我们的harness尽可能相似于AdobeReader如何使用JP2KLib,我们必须做的第一件事就是找到一个PDF文件来触发我们想要的行为.这使我们能够轻松地定位程序的相关部分.

在我们的例子中,我们有大量的PDF来测试我们的产品.我们正则了字符串"/JPXDecode",它是JPEG 2000的PDF过滤器(filter),并使用了最小的示例.我们也可以搜索一个示例文件,或者使用AcrobatPro/Phantom PDF生成一个测试用例.

**Pro Tip 1:** reader有一个沙箱,他有时会使得调试/鉴别分类很麻烦,但是它可以被禁用.https://forums.adobe.com/thread/2110951

**Pro Tip 2:** 我们打开了PageHeap来帮助逆向,因为它有助于跟踪分配的地方和大小.

我们从示例中提取了J2P文件,这样我们就可以脱离PDF wrapper来使用我们的harness.这将用作我们测试输入的harness.

现在我们有了一个最小的工作示例,我们在们在JP2KLib.dll的Load事件上放置了一个断点“SXE ld jp2klib”。当断点命中时，我们在JP2KLib的所有导出函数上放置了一个断点命令。断点命令记录调用堆栈、前几个参数和返回值：

> bm /a jp2klib!\* “.echo callstack; k L5; .echo parameters:;  dc esp L8;  .echo return value: ; pt; ”

我们加载了示例PDF并获得了下面的输出:

![fig5-1.png](https://i.loli.net/2020/07/05/NoiTcMhL2RHvOGW.png)

JP2KLibInitEx是加载JP2KLib之后调用的第一个函数。我们注意到JP2KLibInitEx只使用一个参数。让我们来测试一下:

![fig6-1.png](https://i.loli.net/2020/07/05/ZVP4Jkmer7W39oB.png)

我们可以看到,它是一个大小为0x20的结构,它包含指向AcroRd32.dll中函数的指针.当我们遇到一个未知的函数时,我们不会仓促的逆向它因为我们不知道目标代码是否会使用它.相反,我们将每个地址指向唯一的空函数,我们称之为"NOPX"(其中x是一个数字)

我们现在有足够的信息开始编写我们的harness 架构:

1. 从命令行参数获取输入文件
2. 加载JP2KLib.dll
3. 获取一个指向JP2KLibInitEx的指针，并使用由8个NOP函数组成的结构调用它

![fig7-1.png](https://i.loli.net/2020/07/05/7FBihGpbRuelyfw.png)

我们使用了LOAD_FUNC作为一个简单的宏.我们还有一个NOP(x)宏来创建NOP函数.

![screent.png](https://i.loli.net/2020/07/05/4yvEG6zJawLogdR.png)

我们编译,用sample.jp2运行它,它就能工作了.

让我们继续("g"). 然后我们转到下一个函数JP2KGetMemObjEx,它不带任何参数,所以我们继续调用它,并保存返回值.

下一个函数JP2KDecoptCreate也不带任何参数,因此我们调用它并保存返回值.但是,我们注意到JP2KDecoptCreate内部调用nop4和nop7,这意味着我们需要实现它们.

我们下一步是理解"nop4"的作用.我们在指向"nop4"AcroRd32!CTJPEGDecoderRelease+0xa992的原始指针函数上放置了一个断点并继续执行.

![fig8.png](https://i.loli.net/2020/07/05/4cQr5FRlexAKawq.png)

然后指向:

![fig9-1.png](https://i.loli.net/2020/07/05/3YsFZveNTdHywWq.png)

经过几个step之后.

![fig10-1.png](https://i.loli.net/2020/07/05/wP79WFT46jQhZ2l.png)

因此,nop4是围绕`malloc`的简单包装(thin wrapper).我们在我们的harness中实现了它,并将其替换成"nop4".我们再次为nop7重复了这个过程,并发现了它是`memset`. 我们继续检索,发现nop5和nop6分别是`free`和`memcpy`.

下一个函数是, JP2KDecOptInitToDefaults,用一个参数调用.这是来自JP2KDecoptCreate的返回值,因此我们将这个值传给他.

下一个函数,JP2KImageCreate,不带参数,因此我们调用它并保存返回值.

目前,我们的harness看起来是这样子的:

![fig11-1.png](https://i.loli.net/2020/07/05/H1LpVDRvQI5FJtB.png)

下一个函数是JP2KImageInitDecoderEx,有五!个参数.

我们匹配了5个参数中的3个,他们是JP2KImageCreate, JP2KDecOptCreate 和JP2KGetMemObjEx的返回值.

我们注意到第三个参数指向一个vtable.我们做了和以前一样的工作,创建了一个具有相同大小的结构,指向nop函数.

第二个参数指向另一个结构,只是这次他似乎不包含函数指针.我们决定发送常量`0xbaaddaab`

此时代码如下:

![fig12-1.png](https://i.loli.net/2020/07/05/zb8HIUvj2FmDnrh.png)

我们跑了一下我们的harness,很快就到了nop10.我们在对应函数的AdobeReader中设置一个断点，并得到以下调用堆栈：

![fig13-1.png](https://i.loli.net/2020/07/05/7ruZzCfa8OWSjwe.png)

查看JP2KCodeStm::IsSeekable in IDA：

![fig14-1.png](https://i.loli.net/2020/07/05/OpvhXUKRJ7CcMrj.png)

查看winDbg,我们可以看到在偏移量0x24处的JP2KCodeStm包含我们的vtable,偏移量0x18包含0xbaaddaab。我们可以看到P2KCodeStm::IsSeekable调用了一个函数,将0xbaddaab作为第一个参数传递,因此它基本上是我们的vtable 函数#7的一个简易包装.

一般来说,每个parser都有一点不同,但通常它们使用的输入流可能位于熟悉的文件接口中(比如FILE/ifstream).通常情况下,是某种自定义类型抽象了底层输入流(网络/文件/内存).所以当我们看到JP2KCodeStm是如何被使用时，我们知道我们在看什么。

回到我们的情况下,0xbaaddaab是流对象，vtable函数控制着这个流对象.

我们转移到IDA上,查看了所有其他的JP2KCodeStm::XXX函数

![fig15-1.png](https://i.loli.net/2020/07/05/JIOMuTlyw7Wa2RK.png)

它们都非常相似,因此我们继续创建子的文件对象,并实现了所有必要的方法.生成的代码如下:

![fig16-1.png](https://i.loli.net/2020/07/05/nf9iAPhVdTvUqHu.png)

我们确保检查了JP2KImageInitDecoderEx的返回值,并在错误的时候进行了boiled.在我们的例子中,JP2KImageInitDecoderEx在成功时返回0。我们尝试了几次才能正确实现流函数，但最终得到了我们想要的返回值。

下一个函数,JP2KImageDataCreate,不带参数,其返回值将传递给以下函数JP2KImageGetMaxRes,我们都调用了它们,并继续运行.

我们得到了JP2KImageDecodeTileInterleved函数,有7!个参数.其中3个是来自JPKImageCreate、JP2KImageGetMaxRes和JP2KImageDataCreate的返回值.

在IDA中对AcroRd 32进行修正和观察后，发现第2和第6参数为空。

我们对第四和第五个参数进行观察,得出的结论是它们取决于颜色的深度(8/16),所以我们决定继续fuzz用恒定的深度.

最后我们得到:

![fig17-1.png](https://i.loli.net/2020/07/05/6nphrgXMs1H5yEb.png)

最后,我们调用了JP2KImageDataDesty、JP2KImageDesty和JP2KDecoptDesty函数,以释放我们创建的对象并避免内存泄漏.这对于WinAFL来说至关重要 当模糊迭代比较高的时候.

搞定!我们有了一个可以工作的harness.

在最后一次调整中,我们将初始化代码分离---加载JP2KLib并从解析代码中查找函数.这将提高我们的性能,因此我们不必为每次的模糊迭代初始化花费.我们称这个新功能为"fuzzme".我们还将导出"fuzzme"(你可以在exe文件中导出函数)，因为它比在二进制文件中找到相关偏移量更容易。

趣闻: 当我们在WinAFL测试我们的harness的时候,我们发现WinAFL生成的文件具有相同的魔力. 在深入研究之后,我们发现Adobe使用了不同于libc的SEEK 恒量, 这导致我们混合了SEEK
_SET和SEEK_CUR.

#### ****Fuzzing Methodology****

1. 线束的基本测试
   1. 稳定度
   2. 路径
   3. 超时
2. 启动Fuzzing
3. 初始语料库
4. 初始覆盖
5. Fuzz循环
   1. Fuzz
   2. 检查覆盖/崩溃
   3. Cmin&repeat
6. Triage

#### **Basic Tests for the Harness**

在我们开始一个大的fuzzing 项目之前,我们先做了一些正常的测试,以确保我们不只是在加热服务器. 我们第一件检查的事情是我们的harness在到达新路径,这意味着总路径数量在不断上升.

如果路径计数为0或者几乎为0,则可以检查以下几个缺陷:

* 目标函数是内连在编译器的,导致WinAFL错过目标函数的入口,并使得程序终止,因此WinAFL终止.
* 如果参数的数量不正确,或者调用约定不是默认的,也可能发生这样的情况.
* 超时,有时是时间设置太低,导致fuzzer太快kill了harness. 解决方法是提高timeout.

我们让fuzz跑了几分钟,并检查了fuzz的稳定性.如果稳定性低(低于80%),我们尝试debug这个问题.harness的稳定性是很重要的,因为它会影响fuzz的准确度和性能.

共同的问题：

* 检查随机元素。例如，一些哈希表实现使用随机方法来防止冲突，但这确实不利于覆盖率的准确性。我们只是把随机种子固定在一个恒定的值上。
* 有时，软件对某些全局对象具有缓存。我们通常只是在调用*目标函数*减少这种影响
* 对于Windows 10 64位计算机上的32位目标，堆栈对齐并不总是8字节.这意味着有时*模因*和其他AVX优化代码将采取不同的行动，这确实影响覆盖。一个解决方案是在工具中添加代码以对齐堆叠.

如果上述所有操作都失败了，我们将使用DynamoRIO对控制进行指令跟踪，并对输出进行区分。

#### 启动fuzzing

我们的设置包括一个8-16核和32 GB RAM的VM，运行Windows 10 x64.

我们在RAM磁盘驱动器上使用ImDisk工具包.我们发现，对于快速目标，将测试用例写入磁盘是性能瓶颈。

我们禁用Windows Defender是因为它会损害性能，而且WinAFL生成的一些测试用例是由Windows Defender作为已知的漏洞发现的(“利用漏洞：Win 32/CVE-2010-2889”)。

![fig18-1.png](https://i.loli.net/2020/07/05/hxUSq7yDj4d9P8B.png)

为了提高性能，我们禁用了Windows Indexing Service.

 我们禁用了Windows Update 因为它会干扰fuzzing efforts.(重新启动机器并替换fuzzed的DLLs)

我们为利用过程启用了页面堆，因为它已经证明可以找到我们无法检测到的错误。

我们使用了edge(边)作为coverage type来替代默认basic block(基本块),因为事实证明它在查找bug方面更好,尽管它比基本块慢.

这个是个运行adobe_jp2k harness的示例命令:

AFL-fuzz.exe-i R：\jp2k\in-o R：\jp2k\out-t 20000+-Dc：\DynamoRIO-Windows7.0.0-RC1\bin32-S Slav02--Fuzz_iterations 10000-overage_模块JP2KLib.dll-Target_模块adobe_jp2k.exe-Target_Method fuzzme-Nargs 1-covtype边缘-adobe_jp2k.exe@exe@

(这个写法看起来有一点点迷?)

#### **Initial Corpus**

初始化语料库.

一旦我们有了一个有效harness,我们就为它创建一个harness,通常来自于:

* 网上的语料库([afl corpus](https://lcamtuf.coredump.cx/afl/demo/), [openjpeg-data](https://github.com/uclouvain/openjpeg-data))
* 来自开源文件的测试套件
* 爬 google/duckduckgo
* 语料库来自古老的fuzzing项目

#### **Corpus Minimization**

语料库最小化

使用产生相同覆盖范围的大文件会损害fuzzer的性能.AFL通过使用AFL-cmin最小化语料库来处理这个问题. WinAFL有一个名为winafl-cmin.py的工具端口.

我们获取我们收集的所有文件,并通过winafl-cmin.py运行它们,从而得到一个最小的语料库.

我们至少运行两次winafl-cmin来查看是否得到相同的文件集。如果我们有两个不同的集合，这通常意味着我们的harness中存在不确定性。这是我们试图调查的东西，使用AFL-showmap或其他工具。

一旦我们成功地完成最小化，我们就将文件集保存为我们的初始语料库。

#### **Initial Line Coverage**

初始化覆盖.

现在我们有了一个最小的语料库，我们想看看我们的line coverage。line coverage是指我们实际执行的装配指令。为了获得线路覆盖，我们使用DynamoRIO：

> [dynamoriodir]\bin32\drrun.exe -t drcov — harness.exe testcase

接下来,我们使用IDA的Lighthouse:

![fig19-1.png](https://i.loli.net/2020/07/05/Scifu2aDl6hKzGb.png)

我们注意到initial line coverage,因为它有助于我们评估fuzz的有效性.

#### **Fuzzing Cycle**

下一步是非常直接的:

1. 运行fuzzers
2. 检查coverage和crashes
3. 调查coverage,cmin然后循环

运行fuzzers,不需要什么特殊的东西,只需在上面列出的配置中运行Fuzzers即可。

我们有一个具有以下特点的bot：

1. 所有fuzzers的状态(使用winafl-Whatsapp.py)
2. 每个fuzzer程序的路径图(使用winafl-plot.py)。
3. Crash分类和生成报告(在下一节讨论)

我们再怎么强调自动化这些任务的重要性也不为过。否则，fuzzer是乏味和容易出错的。

我们每隔几个小时就检查一次fuzzers的状态，以及经过一段时间后的路径。如果我们看到这个图是稳定的，我们就试着去调查它的覆盖范围。

![fig20.jpg](https://i.loli.net/2020/07/05/QL3eB4iuyXPJHdr.jpg)

我们复制所有Fuzzer的所有队列，并通过cmin运行它们，并查看IDA中的结果。我们寻找的功能比较大，覆盖面很小。我们试图了解哪些功能与此函数相关，并主动寻找将触发此功能的示例。在JP2K中，这不是很有帮助，但在其他目标，特别是文本格式，这个阶段是一个巨大的胜利！

这个阶段非常重要；在一个例子中，我们添加了一个样本，经过几个小时的模糊处理，它贡献了1.5%的行覆盖率，我们发现了3个新的安全漏洞。

然后，我们重复这个循环，直到我们没有时间或我们没有看到任何覆盖改善。这通常意味着我们必须要么改变目标，要么努力改进harness.

#### **Triage**

一旦我们有了一组导致crash的测试用例,我们就手动检查了crash和每个crsah输入.我们很快改变了策略，因为我们有很多重复。我们开始用[BugId](https://github.com/SkyLined/BugId)若要自动查找重复项并将设置最小化为唯一crash.我们用我们的bot来做这件事情.

![fig21-1.png](https://i.loli.net/2020/07/05/slDUdkH5vF2TEha.png)

#### What We Found

这一策略最终使我们能够在AdobeReader和AdobePro中发现53个关键的bug。

对于iamges、stream decoders和XSLT module等不同的解析器，我们重复了这个过程，得到了以下CVEs列表：

CVE-2018-4985, CVE-2018-5063...

当然，AdobeReader是沙箱式的，而Reader受保护模式大大增加了将沙箱内可利用的崩溃转化为系统危害的复杂性，这通常需要另一个PE攻击，就像在前面提到的in-the-wild exploit所使用的。

在使用WinAFL时，我们遇到了许多bug/缺失的特性。我们增加了对这些新特性的支持，并更新了补丁程序。这包括在Windows 10中添加对App验证器的支持，为工作人员添加CPU亲和力，修复一些bug，并添加一些GUI功能。

You can view the commits here:

Netanel’s commits – https://github.com/googleprojectzero/winafl/commits?author=netanel01

Yoava’s commits – https://github.com/googleprojectzero/winafl/commits?author=yoava333

