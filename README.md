# skr university learning record

## 这是什么?

在大二快大三这年,作为一个零基础的二进制安全学习者跟随sakura师傅学习的记录.

## 学习清单

Week1 我还没进来orz

<details>
<summary>Week 2 SGI STL源码抄读</summary>
> 参考资料:[STL源码剖析](https://www.kancloud.cn/digest/stl-sources/177263)

- [x] ``SGI STL源码抄读``:  进度Allocator->Iterator->base function->vector

- [x] ``思考题1``:操作不恰当时造成的安全问题.

  - vector中erase删除某个元素时,后面的元素会自动向前移动,如果在一个循环中先得到原来的end,但是erase之后,vector的last指针前移,而end却是原来的值,所以会出现野指针.
  - 这个是sad师傅的思路:就是vector的二倍扩充,可能会转移到另外一块内存,而原来的指针指向的地方就会是已经被析构过了的.

- [ ] `待完善`:

  - [ ] asan的使用:用来验证poc

  - [ ] 复习STL:因为当时学的比较仓促,可能还有一些地方并没有理清楚.

    </details>
    
    

<details>
<summary>Week 3~5 编译原理 compiler</summary>

- [x] 哈工大mooc

- [x] cs143 PA2-PA4 

  </details>

等忙完这段时间会总结的