# skr university learning record

## 这是什么?

skr university学习记录.

## 学习清单

开学迟到了一周.

<details>
<summary>Week 2 SGI STL源码抄读</summary>


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
- [ ] `待完善`:
  - [ ] 总结
</details>
<details>
<summary>Week 6~7 杂 </summary>

- [x] 学校大作业
- [x] 退休前的几场CTF 
- [x] 学了一点AFL的知识
</details>
<details>
<summary>Week 8 AFL文件格式和补基础</summary>

- [x] WinAFL : 在看关于GDI+的内容
- [x] csapp的学习
</details>
<details>
<summary>Week9 csapp学习</summary>

- [x] 继续看csapp
- [x] 做了两个简单的lab

