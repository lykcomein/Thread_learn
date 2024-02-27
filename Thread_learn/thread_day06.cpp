//并发三剑客future，promise，async
#include <iostream>
#include <future>
#include <chrono>

//std::async 是一个用于异步执行函数的模板函数，它返回一个 std::future 对象，该对象用于获取函数的返回值。

//定义一个异步任务
std::string fetchDataFromDB(std::string query) {
	//模拟一个异步任务，比如从数据库中获取数据
	std::this_thread::sleep_for(std::chrono::seconds(5));
	return "Data: " + query;
}

void use_async() {
	//使用std::async异步调用fetchDataFromDB
	std::future<std::string> resultFromDB = std::async(std::launch::async, fetchDataFromDB, "Data");

	//在主线程中做其他事情
	std::cout << "Doing something else..." << std::endl;

	//从future对象获取数据
	std::string dbData = resultFromDB.get();
	std::cout << dbData << std::endl;

}
/*
* 在这个示例中，std::async 创建了一个新的线程（或从内部线程池中挑选一个线程）并自动与一个 std::promise 对象相关联。
* std::promise 对象被传递给 fetchDataFromDB 函数，函数的返回值被存储在 std::future 对象中。
* 在使用 std::async 的情况下，我们必须使用 std::launch::async 标志来明确表明我们希望函数异步执行。
* 在主线程中，我们可以使用 std::future::get 方法从 std::future 对象中获取数据。
*/

/*
* async的启动策略
* std::async函数可以接受几个不同的启动策略，这些策略在std::launch枚举中定义。
* 除了std::launch::async之外，还有以下启动策略：
* std::launch::deferred：这种策略意味着任务将在调用std::future::get()或std::future::wait()函数时延迟执行。
* 换句话说，任务将在需要结果时同步执行。
* std::launch::async | std::launch::deferred：这种策略是上面两个策略的组合。
* 任务可以在一个单独的线程上异步执行，也可以延迟执行，具体取决于实现。
* 
* 默认情况下，std::async使用std::launch::async | std::launch::deferred策略。
* 这意味着任务可能异步执行，也可能延迟执行，具体取决于实现。
* 需要注意的是，不同的编译器和操作系统可能会有不同的默认行为。
** 
*/


/*
* std::future::get() 和 std::future::wait()
* 1、std::futurn::get()
* std::future::get() 是一个阻塞调用，用于获取 std::future 对象表示的值或异常。
* 如果异步任务还没有完成，get()会阻塞当前线程，直到任务完成。如果任务已经完成，get()会立即返回任务的结果。
* 重要的是，get() 只能调用一次，因为它会移动或消耗掉 std::future 对象的状态。
* 一旦 get() 被调用，std::future 对象就不能再被用来获取结果。
* 2、std::futurn::wait()
* std::future::wait() 也是一个阻塞调用，但它与 get() 的主要区别在于 wait() 不会返回任务的结果。
* 它只是等待异步任务完成。如果任务已经完成，wait() 会立即返回。
* 如果任务还没有完成，wait() 会阻塞当前线程，直到任务完成。
* 与 get() 不同，wait() 可以被多次调用，它不会消耗掉 std::future 对象的状态
*/

/*
* 总结一下，这两个方法的主要区别在于：
* std::future::get() 用于获取并返回任务的结果，而 std::future::wait() 只是等待任务完成。
* get() 只能调用一次，而 wait() 可以被多次调用。
* 如果任务还没有完成，get() 和 wait() 都会阻塞当前线程，但 get() 会一直阻塞直到任务完成并返回结果，而 wait() 只是在等待任务完成。
*/

/*
* std::packaged_task和std::future是C++11中引入的两个类，它们用于处理异步任务的结果。
* std::packaged_task是一个可调用目标，它包装了一个任务，该任务可以在另一个线程上运行。它可以捕获任务的返回值或异常，并将其存储在std::future对象中，以便以后使用。
* std::future代表一个异步操作的结果。它可以用于从异步任务中获取返回值或异常。
*/

int my_task() {
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::cout << "my task run 5 s" << std::endl;
	return 42;
}

void use_package() {
	//创建一个包装了任务的std::package_tack对象
	std::packaged_task<int()> task(my_task);

	//获取与任务关联的std::future对象
	std::future<int> result = task.get_future();

	//在另一个线程上执行任务
	std::thread t(std::move(task));
	t.detach(); //将线程与主线程分离，以便主线程可以等待任务完成

	//等待任务完成并获取结果
	int value = result.get();
	std::cout << "The result is: " << value << std::endl;

}

/*
* std::promise用于在某一线程中设置某个值或异常
*/

void set_value(std::promise<int> prom) {
	//设置promise的值
	std::this_thread::sleep_for(std::chrono::seconds(5));
	prom.set_value(10);
	std::cout << "promise set value success" << std::endl;
}

void use_promise() {
	//创建一个promise对象
	std::promise<int> prom;
	//获取与promise相关联的future对象
	std::future<int> fut = prom.get_future();
	//在新线程中设置promise的值
	std::thread t(set_value, std::move(prom));
	//t.join();
	//在主线程中获取future的值
	std::cout << "Waiting for the thread to set the value..." << std::endl;
	std::cout << "Value set by the thread: " << fut.get() << std::endl;
	t.join();

}

void set_exception(std::promise<void> prom) {
	try {
		// 抛出一个异常
		throw std::runtime_error("An error occurred!");
	}
	catch (...) {
		// 设置 promise 的异常
		prom.set_exception(std::current_exception());
	}
}

void use_promise_exception() {
	std::promise<void> prom;
	// 获取与 promise 相关联的 future 对象
	std::future<void> fut = prom.get_future();
	// 在新线程中设置 promise 的异常
	std::thread t(set_exception, std::move(prom));
	// 在主线程中获取 future 的异常
	try {
		std::cout << "Waiting for the thread to set the exception...\n";
		fut.get();
	}
	catch (const std::exception& e) {
		std::cout << "Exception set by the thread: " << e.what() << '\n';
	}
	t.join();
}

void use_promise_destruct() {
	std::thread t;
	std::future<int> fut;
	{
		//创建一个promise对象
		std::promise<int> prom;
		//获取与promise相关的future对象
		fut = prom.get_future();
		//在新线程中设置promise的值
		t = std::thread(set_value, std::move(prom));
	}

	//在主线程中获取future的值
	std::cout << "Waiting for the thread to set the value..." << std::endl;
	std::cout << "Value set by the thread: " << fut.get() << std::endl;
	t.join();
}

void myFunction(std::promise<int>&& promise) {
	//模拟一些工作
	std::this_thread::sleep_for(std::chrono::seconds(1));
	promise.set_value(42);
}

//共享型的future
void threadFunction(std::shared_future<int> future) {
	try {
		int result = future.get();
		std::cout << "Result: " << result << std::endl;
	}
	catch (const std::future_error& e) {
		std::cout << "Future error: " << e.what() << std::endl;
	}
}

void use_shared_future() {
	std::promise<int> promise;
	std::shared_future<int> future = promise.get_future();
	std::thread myThread1(myFunction, std::move(promise));//将promise移动到线程中

	//使用share()方法获取新的shared_future对象
	std::thread myThread2(threadFunction, future);

	std::thread myThread3(threadFunction, future);

	std::cout <<"The promis reback: " << future.get() << std::endl;

	myThread1.join();
	myThread2.join();
	myThread3.join();
}
//一个future被移动给两个shared_future是错误的
void use_shared_future_error() {
	std::promise<int> promise;
	std::shared_future<int> future = promise.get_future();

	std::thread myThread1(myFunction, std::move(promise)); // 将 promise 移动到线程中

	// 使用 share() 方法获取新的 shared_future 对象  

	std::thread myThread2(threadFunction, std::move(future));

	std::thread myThread3(threadFunction, std::move(future));

	myThread1.join();
	myThread2.join();
	myThread3.join();
}

void may_throw() {
	//这里我们抛出一个异常。在实际的程序中，这可能在任何地方发生
	throw std::runtime_error("Oops, something went wrong!");
}

void use_funture_exception() {
	//创建一个异步任务
	std::future<void> result(std::async(std::launch::async, may_throw));

	try {
		//获取结果(如果在获取结果时发生了一场，那么会重新抛出异常)
		result.get();
	}
	catch (const std::exception& e) {
		//捕获并打印异常
		std::cerr << "Caught exception: " << e.what() << std::endl;
	}
}

int main() {
	//use_async();
	//my_task();
	//use_package();
	//use_promise();
	//use_promise_exception();
	//use_shared_future();
	use_funture_exception();
	return 0;
}