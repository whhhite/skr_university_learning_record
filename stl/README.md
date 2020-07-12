## 这是什么

对SGI_STL的学习

## 一些笔记

#### 认识headers 版本 重要资源

所谓GP,泛型编程,就是使用template(模板)为主要工具来编写程序.

STL正是泛型编程最成功的作品.

c++ standard library

c++标准库

standard template library

STL,标准模板库

重要网页 

* CPlusPlus.com

* gcc.gnu.org

#### STL体系结构基础介绍

STL六大部件(Components):

* 容器(Containers): 各种数据结构，如Vector,List,Deque,Set,Map,用来存放数据，STL容器是一种Class Template,就体积而言，这一部分很像冰山载海面的比率。

* 分配器(Allocators):负责空间配置与管理，从实现的角度来看，配置器是一个实现了动态空间配置、空间管理、空间释放的Class Template。

* 算法(Algorithms):各种常用算法如Sort,Search,Copy,Erase,从实现的角度来看，STL算法是一种Function Templates。

* 迭代器(Iterators):扮演容器与算法之间的胶合剂，是所谓的“泛型指针”，共有五种类型，以及其它衍生变化，从实现的角度来看，迭代器是一种将：Operators*,Operator->,Operator++,Operator--等相关操作予以重载的Class Template。所有STL容器都附带有自己专属的迭代器——是的，只有容器设计者才知道如何遍历自己的元素，原生指针（Native pointer）也是一种迭代器。

* 配接器(Adapters):一种用来修饰容器（Containers）或仿函数（Functors）或迭代器（Iterators）接口的东西，例如：STL提供的Queue和Stack，虽然看似容器，其实只能算是一种容器配接器，因为 它们的底部完全借助Deque，所有操作有底层的Deque供应。改变Functor接口者，称为Function Adapter;改变Container接口者，称为Container Adapter;改变Iterator接口者，称为Iterator Adapter。配接器的实现技术很难一言蔽之，必须逐一分析。

* 仿函数(Functors) :行为类似函数，可作为算法的某种策略（Policy）,从实现的角度来看，仿函数是一种重载了Operator()的Class 或 Class Template。一般函数指针可视为狭义的仿函数。

示意图:

![1.png](https://i.loli.net/2020/07/11/PkhuACEXcD8iT1U.png)

![2.png](https://i.loli.net/2020/07/11/1XxeFvOyUnc76md.png)

![3.png](https://i.loli.net/2020/07/11/zo21ayCYk3v7mbG.png)



关于for的一种用法:

![4](https://i.loli.net/2020/07/11/CGOHJa7NAlTrFUL.png)

auto的用法:

![5.png](https://i.loli.net/2020/07/11/wax7JfZhUQjIuqX.png)

#### 容器之分类与各类测试

![6.png](https://i.loli.net/2020/07/11/DS7WpbQ98J2iTcG.png)

就是顺序与链式 然后unorder就是不定序

set 和map 没规定是什么 但是红黑树(平衡树 来避免单臂过长的最坏情况)好,所以用红黑树

set 的key和value是一起的 而map 的key和value是分开的

muti说明key 可以重复



提到了写的demo的两种风格

namespace与include 和变量不缩进



#### 分配器之测试

_gnu_cxx::

不建议直接使用分配器

因为deallocate借几个就要还几个

#### OOP(面向对象编程)VS GP(泛式编程)



OOP企图将datas和methods关联在一起

而GP却是将datas和method分开来.

所有algorithms,其内容最终涉及元素本身的操作,无非就是比大小.

![7.png](https://i.loli.net/2020/07/11/MRWa3GvmyqDt1kN.png)



#### 技术基础:操作符重载and模板(泛化,全特化,偏特化)

![8.png](https://i.loli.net/2020/07/11/7AmLkf2wdHavUQM.png)

特化:

![9.png](https://i.loli.net/2020/07/11/O43TDKq9JinVQGL.png)

![10.png](https://i.loli.net/2020/07/11/Wv9ZN5hHDuJEPnr.png)

数量的局部与范围的局部

![11.png](https://i.loli.net/2020/07/11/MsR7rNLiSGha6uE.png)



#### 容器之间的实现关系与分类

![12.png](https://i.loli.net/2020/07/11/yfWKrlAb1kxCXRt.png)



#### 迭代器的设计原则和Iterator Traits的作用与设计

Iterators_traits 萃取机

![13.png](https://i.loli.net/2020/07/11/C96FmWIfnuiVJYa.png)

![14.png](https://i.loli.net/2020/07/11/dXY7f6aNKDbvuhV.png)

![15.png](https://i.loli.net/2020/07/11/9WQ856sBmEhrOnV.png)

![16.png](https://i.loli.net/2020/07/11/oMA34RsTYJeHh6x.png)





