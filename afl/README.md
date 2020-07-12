## 学习AFL过程看过的文章.



* 了解AFL

[AFL漏洞挖掘技术漫谈（一）：用AFL开始你的第一次Fuzzing](https://www.freebuf.com/articles/system/191543.html)

[AFL漏洞挖掘技术漫谈（二）：Fuzz结果分析和代码覆盖率](https://www.freebuf.com/articles/system/197678.html)



* 具体使用 

[模糊测试工具WinAFL使用指南](https://www.freebuf.com/articles/system/216437.html)

[Fuzz 工具 WinAFL 的使用感受 - 对 FreeImage 图片解析库进行模糊测试](https://bbs.pediy.com/thread-255162.htm)

这两篇其实是一个作者写的,但是内容是互补的,也不知道为什么要这么写.

[使用 WinAFL 对 MSXML6 库进行模糊测试](https://www.anquanke.com/post/id/86905)



* 国外文章

[50 CVEs in 50 Days: Fuzzing Adobe Reader](https://research.checkpoint.com/2018/50-adobe-cves-in-50-days/)

↑这一篇我做了个中文翻译 便于以后观看

[Investigating Windows Graphics Vulnerabilities: A Reverse Engineering and Fuzzing Story](https://www.ixiacom.com/company/blog/investigating-windows-graphics-vulnerabilities-reverse-engineering-and-fuzzing-story)

这篇文章分析的是CVE-2019-0618 GDIplus dll 造成的RCE

核心方法是通过diff修补前后两个版本来寻找漏洞修补的地方,再根据这个特质写harness以及生成一些样本.

* 漏洞分析

[CVE-2020-1225 Microsoft Office Excel HTML and XML Table Code Execution Vulnerability](https://talosintelligence.com/vulnerability_reports/TALOS-2020-1045)

[CVE-2020-1226 Microsoft Office Excel PivotField code execution vulnerability](https://talosintelligence.com/vulnerability_reports/TALOS-2020-1027)

两个都是同一洞 原因看起来也是差不多的.

大概是ntdll!里面的RtlpAllocateHeap模块造成的.之后可以试着用上面的那个方法研究.

应该是 HTML/XML标记的XLS文件.


