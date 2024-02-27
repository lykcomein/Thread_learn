/*
* unique_lock
*/
#include <iostream>
#include <mutex>
#include <map>
#include <shared_mutex>


//unique_lock�÷�
/*
* unique_lock��lock_guard�����÷���ͬ������ʱĬ�ϼ���������ʱĬ�Ͻ���
* ��unique_lock�и��ô����ǿ����ֶ�����
*/
std::mutex mtx;
int shared_data = 0;
void use_unique() {
	//unique_lock�����Զ�������Ҳ�����ֶ�����
	std::unique_lock<std::mutex> lock(mtx);
	std::cout << "lock sucess" << std::endl;
	shared_data++;
	lock.unlock();
}

//�ж��Ƿ�ռ����
void owns_lock() {
	//lock���Զ�������Ҳ���ֶ�����
	std::unique_lock<std::mutex> lock(mtx);
	shared_data++;
	if (lock.owns_lock()) {
		std::cout << "owns lock" << std::endl;
	}
	else {
		std::cout << "doesn't own lock" << std::endl;
	}

	lock.unlock();
	if (lock.owns_lock()) {
		std::cout << "owns lock" << std::endl;
	}
	else {
		std::cout << "doesn't own lock" << std::endl;
	}
}

//unique_lock�����ӳټ���
void defer_lock() {
	//�ӳټ���
	std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
	//���Լ���
	lock.lock();
	//�����Զ�����������Ҳ�����ֶ�����
	lock.unlock();
}

//ͬʱʹ��owns��defer
void use_own_defer() {
	std::unique_lock<std::mutex> lock(mtx);
	//�ж��Ƿ�����
	if (lock.owns_lock()) {
		std::cout << "Main thread has the lock." << std::endl;
	}
	else {
		std::cout << "Main thread does not have the lock." << std::endl;
	}

	std::thread t([]() {
		std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
		//�ж��Ƿ�����
		if (lock.owns_lock()) {
			std::cout << "Thread has the lock." << std::endl;
		}
		else {
			std::cout << "Thread does not have the lock." << std::endl;
		}
		//����
		lock.lock();
		//�ж��Ƿ�����
		if (lock.owns_lock()) {
			std::cout << "Thread has the lock." << std::endl;
		}
		else {
			std::cout << "Thread does not have the lock." << std::endl;
		}
		//����
		lock.unlock();
		});
	t.join();
	//����������������, ���ǳ������������Ϊ���̻߳Ῠ�ڼ������߼���(mtx�Ѿ������߳���ס��)��
	// ��Ϊ���߳�δ�ͷ����������߳��ֵȴ����߳��˳���������������ס
}
//ͬ��֧����������
void use_own_adopt() {
	mtx.lock();
	std::unique_lock<std::mutex> lock(mtx, std::adopt_lock);
	if (lock.owns_lock()) {
		std::cout << "owns lock" << std::endl;
	}
	else {
		std::cout << "does not have the lock" << std::endl;
	}
	lock.unlock();
}

//ʵ��֮ǰ�Ľ�������
int a = 10;
int b = 99;
std::mutex mtx1;
std::mutex mtx2;

void safe_swap() {
	std::lock(mtx1, mtx2);
	std::unique_lock<std::mutex> lock1(mtx1, std::adopt_lock);
	std::unique_lock<std::mutex> lock2(mtx2, std::adopt_lock);
	std::swap(a, b);
	//�����÷�
	//mtx1.unlock();
	//mtx2.unlock();
	//һ��mutex��unique_lock�����������ͷŵĲ����ͽ���unique_lock��
	//���ܵ���mutex�����ͽ�������Ϊ����ʹ��Ȩ�Ѿ�����unique_lock��
}
void safe_swap2() {
	std::unique_lock<std::mutex> lock1(mtx1, std::defer_lock);
	std::unique_lock<std::mutex> lock2(mtx2, std::defer_lock);
	//��Ҫ��lock1��lock2����
	std::lock(lock1, lock2);
	//�����÷�
	//std::lock(mtx1, mtx2);
	std::swap(a, b);
}

void test_swap1() {
	std::thread t1(safe_swap);
	std::thread t2(safe_swap);
	//std::thread t3(safe_swap);
	t1.join();
	t2.join();
	//t3.join();
}
void test_swap2() {
	std::thread t1(safe_swap2);
	std::thread t2(safe_swap2);
	//std::thread t3(safe_swap);
	t1.join();
	t2.join();
	//t3.join();
}

//ת�ƻ���������Ȩ
//����������֧��move����������unique_lock֧��
std::unique_lock<std::mutex> get_lock() {
	std::unique_lock<std::mutex> lock(mtx);
	shared_data++;
	return lock;
}

void use_return() {
	std::unique_lock<std::mutex> lock(get_lock());
	shared_data++;
}


//�����ȱ�ʾ�����ľ�ϸ�̶ȡ�
//һ����������Ҫ�㹻���Ա�֤������סҪ���ʵĹ������ݡ�
//һ����������Ҫ�㹻С���Ա�֤�ǹ�������ݲ�����סӰ�����ܡ�
void precision_lock() {
	std::unique_lock<std::mutex> lock(mtx);
	shared_data++;
	lock.unlock();
	//���漰�������ݵĺ�ʱ������Ҫ��������ִ��
	std::this_thread::sleep_for(std::chrono::seconds(1));
	lock.lock();
	shared_data++;
}

//������
//C++ 17 ��׼shared_mutex
//C++14  �ṩ�� shared_time_mutex
//C++11 ���������⣬��ʹ�ÿ�������boost��



int main() {
	//owns_lock();
	//use_own_defer();
	std::cout << "a: " << a << std::endl;
	std::cout << "b: " << b << std::endl;
	test_swap2();
	std::cout << "a: " << a << std::endl;
	std::cout << "b: " << b << std::endl;
	return 0;
}