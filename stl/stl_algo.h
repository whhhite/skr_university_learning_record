#ifndef WHITE_STL_STL_ALGO_H
#define WHITE_STL_STL_ALGO_H

#include "stl_config.h"

__STL_BEGIN_NAMESPACE
//����ָ��Χ [first, last) ���׸���С�ڣ������ڻ���ڣ� value ��Ԫ�صĵ������������Ҳ�������Ԫ���򷵻� last ��
// Binary search (lower_bound, upper_bound, equal_range, binary_search).

template<class _ForwardIter, class _Tp, class _Distance>
_ForwardIter __lower_bound(_ForwardIter __first, _ForwardIter __last,
	const _Tp& __val, _Distance*) {
	_Distance __len = 0;
	distance(__first, __last, __len);
	_Distance __half;
	_ForwardIter __middle;

	while (__len > 0) {
		__half = __len >> 1;
		__middle = __first;
		advance(__middle, __half);
		if (*__middle < __val) {
			__first = __middle;
			++__first;
			__len = __len - __half - 1;
		}
		else
			__len = __half;
	}
	return __first;
}

template<class _ForwardIter, class _Tp>
inline _ForwardIter lower_bound(_ForwardIter __first, _ForwardIter __last,
	const _Tp& __val) {
	return __lower_bound(__first, __last, __val,
		__DISTANCE_TYPE(__first));
}

template<class _ForwardIter, class _Tp, class _Compare, class _Distance>
_ForwardIter __lower_bound(_ForwardIter __first, _ForwardIter __last,
	const _Tp& __val, _Compare __comp, _Distance*) {
	_Distance __len = 0;
	distance(__first, __last, __len);
	_Distance __half;
	_ForwardIter __middle;

	while (__len > 0) {
		__half = __len >> 1;
		__middle = __first;
		advance(__middle, __half);
		if (__comp(*__middle, __val)) {
			__first = __middle;
			++__first;
			__len = __len - __half - 1;
		}
		else
			__len = __half;
	}
	return __first;
}

template<class _ForwardIter, class _Tp, class _Compare>
inline _ForwardIter lower_bound(_ForwardIter __first, _ForwardIter __last,
	const _Tp& __val, _Compare __comp) {
	return __lower_bound(__first, __last, __val, __comp,
		__DISTANCE_TYPE(__first));
}
__STL_END_NAMESPACE
#endif
