//ͨ�������뺯��ʽ��̣���߿��������Ч��
#include <iostream>
#include <list>
#include <algorithm>
#include <future>
#include "Thread_pool.h"

//c++�汾�Ŀ�������
template<typename T>
void quick_sort_recursive(T arr[], int start, int end) {
	if (start >= end) return;
	T key = arr[start];
	int left = start, right = end;
	while (left < right) {
		while (arr[right] >= key && left < right) right--;
		while (arr[left] <= key && left < right) left++;
		std::swap(arr[left], arr[right]);
	}
	if (arr[left] < key) {
		std::swap(arr[left], arr[start]);
	}
	quick_sort_recursive(arr, start, left - 1);
	quick_sort_recursive(arr, left + 1, end);
}

template<typename T>
void quick_sort(T arr[], int len) {
	quick_sort_recursive(arr, 0, len - 1);
}

//ʵ��list�����еĿ�������
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input) {
	if (input.empty()) {
		return input;
	}
	std::list<T> result;

	//�� ��input�еĵ�һ��Ԫ�ط���result�У����ҽ����һ��Ԫ�ش�input��ɾ��
	//����splice��input.begin()��ָλ�õ�Ԫ��ת�Ƶ�result.begin()��ָ��λ��
	result.splice(result.begin(), input, input.begin());

	//�� ȡresult�ĵ�һ��Ԫ�أ����������Ԫ�����и�и�input�е��б�
	T const& pivot = *result.begin();

	//�� std::partition ��һ����׼�⺯�������ڽ������������е�Ԫ�ذ���ָ�����������з�����
	// ʹ������������Ԫ�����ڲ�����������Ԫ��֮ǰ��
	// ���Ծ�������divide_pointָ�����input�е�һ�����ڵ���pivot��Ԫ��
	auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t) {
		return t < pivot;
		});

	//�� ��С��pivot��Ԫ�ط��뵽lower_part��
	//����splice��input.begin()��divide_point��Ԫ��(����ҿ�)�ŵ�lower_part��lower_part.end()ǰ
	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	//�� ��lower_part���ݸ�sequential_quick_sort ����һ���µ�����Ĵ�С���������
	//lower_part �ж���С��divide_point��ֵ
	auto new_lower(sequential_quick_sort(std::move(lower_part)));

	//�� ʣ���input�б��ݸ�sequential_quick_sort�ݹ���ã�input�ж��Ǵ���divide_point��ֵ��
	auto new_higher(sequential_quick_sort(std::move(input)));

	//�ߵ���ʱnew_higher��new_lower���Ǵ�С��������õ��б�
	//��new_higher ƴ�ӵ�result��β��
	//splice���ｫnewhigher�ڵ�Ԫ��ȫ���ƶ���result�У�result.end()ǰ
	result.splice(result.end(), new_higher);

	//��new_lowerƴ�ӵ�result��ͷ��
	result.splice(result.begin(), new_lower);

	return result;
}

//���а汾
template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
	if (input.empty()) {
		return input;
	}
	std::list<T> result;
	result.splice(result.begin(), input, input.begin());
	T const& pivot = *result.begin();
	auto divide_point = std::partition(input.begin(), input.end(),
		[&](T const& t) {
			return t < pivot;
		});
	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	//����Ϊlower_part�Ǹ��������Բ��в������������߼����������������future������(ÿ���߳��е�lower_part���ǲ�ͬ��)
	std::future<std::list<T>> new_lower(std::async(&parallel_quick_sort<T>, std::move(lower_part)));

	//����lower_part��ͬ��lower_part��ÿ���߳��ж����´����ı�������input�������̶߳�����ı��������ʹ�ò��в��������ܱ�֤input�Ĳ���˳��
	auto new_higher(parallel_quick_sort(std::move(input)));
	result.splice(result.end(), new_higher);
	result.splice(result.begin(), new_lower.get());
	return result;
}

void test_sequential_quick() {
	std::list<int> numlists = { 6,1,0,7,5,2,9,-1 };
	auto sort_result = sequential_quick_sort(numlists);
	std::cout << "sort result is ";
	for (auto iter = sort_result.begin(); iter != sort_result.end(); iter++) {
		std::cout << " " << (*iter);
	}
	std::cout << std::endl;
}

void test_quick_sort() {
	int num_arr[] = { 6,1,0,7,5,2,9,-1 };
	int length = sizeof(num_arr) / sizeof(int);
	quick_sort(num_arr, length);
	std::cout << "sort result is ";
	for (int i = 0; i < length; i++) {
		std::cout << " " << num_arr[i];
	}
	std::cout << std::endl;
}

void test_parallen_sort() {
	std::list<int> numlists = { 6,1,0,7,5,2,9,-1 };
	auto sort_result = parallel_quick_sort(numlists);
	std::cout << "sorted result is ";
	for (auto iter = sort_result.begin(); iter != sort_result.end(); iter++) {
		std::cout << " " << (*iter);
	}
	std::cout << std::endl;
}

//�̳߳ذ汾
//���а汾
template<typename T>
std::list<T> thread_pool_quick_sort(std::list<T> input) {
	if (input.empty()) {
		return input;
	}
	std::list<T> result;
	result.splice(result.begin(), input, input.begin());
	T const& pivot = *result.begin();
	auto divide_point = std::partition(input.begin(), input.end(),
		[&](T const& t) {
			return t < pivot;
		});
	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	//����Ϊlower_part�Ǹ��������Բ��в������������߼���������Ͷ�ݸ��̳߳ش���
	auto new_lower = ThreadPool::instance().commit(&parallel_quick_sort<T>, std::move(lower_part));

	auto new_higher(parallel_quick_sort(std::move(input)));
	result.splice(result.end(), new_higher);
	result.splice(result.begin(), new_lower.get());

	return result;
}

void test_thread_pool_sort() {
	std::list<int> numlists = { 6,1,0,7,5,2,9,-1 };
	auto sort_result = thread_pool_quick_sort(numlists);
	std::cout << "sorted result is ";
	for (auto iter = sort_result.begin(); iter != sort_result.end(); iter++) {
		std::cout << " " << (*iter);
	}
	std::cout << std::endl;
}

int main() {
	test_quick_sort();
	test_sequential_quick();
	test_parallen_sort();
	test_thread_pool_sort();

	return 0;
}