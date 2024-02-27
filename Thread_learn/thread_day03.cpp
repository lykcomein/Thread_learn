/*
* unique_lock
*/
#include <iostream>
#include <mutex>
#include <map>
#include <shared_mutex>


//unique_lock用法
/*
* unique_lock与lock_guard基本用法相同，构造时默认加锁，析构时默认解锁
* 但unique_lock有个好处就是可以手动解锁
*/
std::mutex mtx;
int shared_data = 0;
void use_unique() {
	//unique_lock可以自动解锁，也可以手动解锁
	std::unique_lock<std::mutex> lock(mtx);
	std::cout << "lock sucess" << std::endl;
	shared_data++;
	lock.unlock();
}

//判断是否占有锁
void owns_lock() {
	//lock可自动解锁，也可手动解锁
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

//unique_lock可以延迟加锁
void defer_lock() {
	//延迟加锁
	std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
	//可以加锁
	lock.lock();
	//可以自动析构解锁，也可以手动解锁
	lock.unlock();
}

//同时使用owns与defer
void use_own_defer() {
	std::unique_lock<std::mutex> lock(mtx);
	//判断是否有锁
	if (lock.owns_lock()) {
		std::cout << "Main thread has the lock." << std::endl;
	}
	else {
		std::cout << "Main thread does not have the lock." << std::endl;
	}

	std::thread t([]() {
		std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
		//判断是否有锁
		if (lock.owns_lock()) {
			std::cout << "Thread has the lock." << std::endl;
		}
		else {
			std::cout << "Thread does not have the lock." << std::endl;
		}
		//加锁
		lock.lock();
		//判断是否有锁
		if (lock.owns_lock()) {
			std::cout << "Thread has the lock." << std::endl;
		}
		else {
			std::cout << "Thread does not have the lock." << std::endl;
		}
		//解锁
		lock.unlock();
		});
	t.join();
	//上述代码回依次输出, 但是程序会阻塞，因为子线程会卡在加锁的逻辑上(mtx已经被主线程锁住了)，
	// 因为主线程未释放锁，而主线程又等待子线程退出，导致整个程序卡住
}
//同样支持领养操作
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

//实现之前的交换代码
int a = 10;
int b = 99;
std::mutex mtx1;
std::mutex mtx2;

void safe_swap() {
	std::lock(mtx1, mtx2);
	std::unique_lock<std::mutex> lock1(mtx1, std::adopt_lock);
	std::unique_lock<std::mutex> lock2(mtx2, std::adopt_lock);
	std::swap(a, b);
	//错误用法
	//mtx1.unlock();
	//mtx2.unlock();
	//一旦mutex被unique_lock管理，加锁和释放的操作就交给unique_lock，
	//不能调用mutex加锁和解锁，因为锁的使用权已经交给unique_lock了
}
void safe_swap2() {
	std::unique_lock<std::mutex> lock1(mtx1, std::defer_lock);
	std::unique_lock<std::mutex> lock2(mtx2, std::defer_lock);
	//需要用lock1，lock2加锁
	std::lock(lock1, lock2);
	//错误用法
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

//转移互斥量所有权
//互斥量本身不支持move操作，但是unique_lock支持
std::unique_lock<std::mutex> get_lock() {
	std::unique_lock<std::mutex> lock(mtx);
	shared_data++;
	return lock;
}

void use_return() {
	std::unique_lock<std::mutex> lock(get_lock());
	shared_data++;
}


//锁粒度表示加锁的精细程度。
//一个锁的粒度要足够大，以保证可以锁住要访问的共享数据。
//一个锁的粒度要足够小，以保证非共享的数据不被锁住影响性能。
void precision_lock() {
	std::unique_lock<std::mutex> lock(mtx);
	shared_data++;
	lock.unlock();
	//不涉及共享数据的耗时操作不要放在锁内执行
	std::this_thread::sleep_for(std::chrono::seconds(1));
	lock.lock();
	shared_data++;
}

//共享锁
//C++ 17 标准shared_mutex
//C++14  提供了 shared_time_mutex
//C++11 无上述互斥，想使用可以利用boost库



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