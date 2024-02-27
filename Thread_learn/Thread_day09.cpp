//async源码分析，以及各种问题
#include <iostream>
#include <thread>
#include <memory>
#include <functional>
#include <future>

//当函数返回一个类类型的局部变量时会先调用移动构造，如果没有移动构造再调用拷贝构造
//优先按照移动构造的方式返回局部的类对象，有一个好处就是可以返回一些只支持移动构造的类型

std::unique_ptr<int> ReturnUniquePtr() {
	std::unique_ptr<int> uq_ptr = std::make_unique<int>(100);
	return uq_ptr;
}

std::thread ReturnThread() {
	std::thread t([]() {
		int i = 0;
		while (true) {
			std::cout << "i is " << i << std::endl;
			i++;
			if (i == 5) {
				break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		});
	return t;
}

void ChangeValue() {
	int m = 100;
	std::thread t1{ [](int& rm) {
		rm++;
}, std::ref(m) };
	t1.join();
}


void ThreadOp() {

	std::thread t1([]() {
		int i = 0;
		while (i < 5) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			i++;
		}
		});

	std::thread t2([]() {
		int i = 0;
		while (i < 10) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			i++;
		}
		});

	//不能将一个线程归属权绑定给一个已经绑定线程的变量，否则会触发terminate导致崩溃
	t1 = std::move(t2);
	t1.join();
	t2.join();
}


//在主函数调用BlockAsync()， 发现async并没有异步执行任务，而是按次序输出
//因为async返回一个右值类型的future，无论左值还是右值，future都要被析构，因为其处于一个局部作用域{}中。
//当编译器执行到}时会触发future析构。但是future析构要保证其关联的任务完成，所以需要等待任务完成future才被析构，
//所以也就成了串行的效果了。
void BlockAsync() {
	std::cout << "begin block async" << std::endl;
	{
		std::async(std::launch::async, []() {
			std::this_thread::sleep_for(std::chrono::seconds(3));
			std::cout << "std::async called " << std::endl;
			});
	}
	std::cout << "end block async" << std::endl;
}

//下面是一种死锁情况
//下面程序主线程输出”DeadLock begin “加锁，此时async启动一个线程，那么lambda表达式会先输出”std::async called “.
//但是在子线程中无法加锁成功，因为主线程没有释放锁。而主线程无法释放锁，因为主线程要等待async执行完
//由于futures处于局部作用域，即将析构，而析构又要等待任务完成，任务需要加锁，所以永远完成不了，这样就死锁了
void DeadLock() {
	std::mutex mtx;
	std::cout << "DeadLock begin " << std::endl;
	std::lock_guard<std::mutex> dklock(mtx);
	{
		std::future<void> futures = std::async(std::launch::async, [&mtx]() {
			std::cout << "std::async called " << std::endl;
			std::lock_guard<std::mutex> dklock(mtx);
			std::cout << "async working..." << std::endl;
			});
	}
	std::cout << "DeadLock end " << std::endl;
}

//下面实现如下需求
/*
* 1、func1 中要异步执行asyncFunc函数。
* 2、func2中先收集asyncFunc函数运行的结果，只有结果正确才执行
* 3、func1启动异步任务后继续执行，执行完直接退出不用等到asyncFunc运行完
*/

int asyncFunc() {
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "this is asyncFunc" << std::endl;
	return 0;
}

void func1(std::future<int>& future_ref) {
	std::cout << "this is func1 " << std::endl;
	future_ref = std::async(std::launch::async, asyncFunc);
}

void func2(std::future<int>& future_ref) {
	std::cout << "this is func2" << std::endl;
	auto future_res = future_ref.get();
	if (future_res == 0) {
		std::cout << "get asyncFunc result success !!" << std::endl;
	}
	else {
		std::cout << "get asyncFunc result failed !" << std::endl;
		return;
	}
}
//这里保证func1与func2使用的是future的引用，这样func1内不会因为启动async而阻塞
void first_method() {
	std::future<int> future_tmp;
	func1(future_tmp);
	func2(future_tmp);
}


int main(){

	//auto rt_ptr = ReturnUniquePtr();
	//std::cout << "rt_ptr value is " << *rt_ptr << std::endl;
	//std::thread rt_thread = ReturnThread();
	//rt_thread.join();

	//ThreadOp();

	BlockAsync();

	return 0;
}