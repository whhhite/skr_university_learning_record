#ifndef WHITE_STL_STL_ALLOC_H
#define WHITE_STL_STL_ALLOC_H
/*对象构造前的空间配置和对象析构后的空间释放，由stl_alloc.h负责。
考虑到小型区块可能造成的内存破碎问题，SGI设计了双层级配置器。
第一级配置器用于处理大于128bytes的情况, 即申请大的内存，第二级配置器用于申请小内存。*/
#include <cstddef>
#include <cstdlib>
#include <cstdio>

#include "stl_config.h"
#include "stl_construct.h"
__STL_BEGIN_NAMESPACE
template<int __inst>
//第一级配置器__malloc_alloc_template:异常处理
class __malloc_alloc_template {
public:
	//申请n个大小的空间
	static void* allocate(size_t __n) {
		void* __result = malloc(__n);
		if (0 == __result) {
			fprintf(stderr, "out of memory\n");
			exit(1);
		}
		return __result;
	}
	//释放空间
	static void deallocate(void* __p, size_t /* __n */) {
		free(__p);
	}
	//重新申请空间
	static void* reallocate(void* __p, size_t/* old_sz */, size_t __new_sz) {
		void* __result = realloc(__p, __new_sz);
		if (0 == __result) {
			fprintf(stderr, "out of memory\n");
			exit(1);
		}
		return __result;
	}
};
//直接将参数inst指定为0
typedef __malloc_alloc_template<0> malloc_alloc;

//对其他适配器(如__Alloc::allocate)的一个简单封装
template<class _Tp, class _Alloc>
class simple_alloc {
public:
	static _Tp* allocate(size_t __n) {
		return 0 == __n ? 0 : (_Tp*)_Alloc::allocate(__n * sizeof(_Tp));
	}

	static _Tp* allocate(void) {
		return (_Tp*)_Alloc::allocate(sizeof(_Tp));
	}

	static void deallocate(_Tp* __p, size_t __n) {
		if (0 != __n) {
			_Alloc::deallocate(__p, __n * sizeof(_Tp));
		}
	}

	static void deallocate(_Tp* __p) {
		_Alloc::deallocate(__p, sizeof(_Tp));
	}
};

typedef malloc_alloc alloc;

__STL_END_NAMESPACE
#endif
