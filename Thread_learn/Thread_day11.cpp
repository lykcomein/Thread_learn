//内存顺序实现内存模型

/*
* 三种顺序模型：
* Sequencial consistent ordering：实现同步, 且保证全局顺序一致 (single total order) 的模型. 是一致性最强的模型, 也是默认的顺序模型.
* Acquire-release ordering：实现同步, 但不保证保证全局顺序一致的模型.
* Relaxed ordering：不能实现同步，只能保证原子性的模型
*/

#include <iostream>
#include <atomic>
#include <thread>
#include <cassert>
#include <vector>
#include <algorithm>
#include <mutex>
#include <memory>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
	x.store(true, std::memory_order_seq_cst); //1
	y.store(true, std::memory_order_seq_cst); //2
}

void read_y_then_x() {
	while (!y.load(std::memory_order_seq_cst)) {//3
		std::cout << "y load false" << std::endl;
	}

	if (x.load(std::memory_order_seq_cst)) {//4
		++z;
	}
}
/*
* 上面的代码x和y采用的是memory_order_seq_cst, 所以当线程t2执行到3处并退出循环时我们可以断定y为true，
* 因为是全局一致性顺序，所以线程t1已经执行完2处将y设置为true，那么线程t1也一定执行完1处代码并对t2可见，
* 所以当t2执行至4处时x为true，那么会执行z++保证z不为零，从而不会触发断言。
*/
void TestOrderSeqCst() {
	std::thread t1(write_x_then_y);
	std::thread t2(read_y_then_x);
	t1.join();
	t2.join();
	assert(z.load() != 0);
}

void TestOrderRelaxed() {
	std::atomic<bool> rx, ry;
	std::thread t1([&]() {
		rx.store(true, std::memory_order_relaxed); //1
		ry.store(true, std::memory_order_relaxed); //2
		});

	std::thread t2([&]() {
		while (!ry.load(std::memory_order_relaxed)); //3
		assert(rx.load(std::memory_order_relaxed)); //4
		});

	t1.join();
	t2.join();
}

/*
* 对原子变量的 load 可以使用 memory_order_acquire 内存顺序. 这称为 acquire 操作.
* 对原子变量的 store 可以使用 memory_order_release 内存顺序. 这称为 release 操作.
* read-modify-write 操作即读 (load) 又写 (store),
* 它可以使用 memory_order_acquire, memory_order_release 和 memory_order_acq_rel:
*/

/*
* ry.store使用的是std::memory_order_release, ry.load使用的是std::memory_order_acquire.
* t1执行到2将ry 设置为true, 因为使用了Acquire-release 顺序，
* 所以 t2 执行到3时读取ry为true， 因此2和3 可以构成同步关系。
* 又因为单线程t1内 1 sequence before 2,所以1 happens-before 3.
* 因为单线程t2内 3 sequence before 4. 所以 1 happens-before 4.
* 可以断定4 不会触发断言。
*/
void TestReleaseAcquire() {
	std::atomic<bool> rx, ry;

	std::thread t1([&]() {
		rx.store(true, std::memory_order_relaxed); //1
		ry.store(true, std::memory_order_release); //2
		});

	std::thread t2([&]() {
		while (!ry.load(std::memory_order_acquire)); //3
		assert(rx.load(std::memory_order_relaxed)); //4
		});

	t1.join();
	t2.join();
}

//t3在yd为true时才会退出，因此有两种情况：
//一种是yd是1，一种是yd是2；
//当yd是2时，可能出现t2(3)执行结束，(1)还没有执行结束的情况，
//此时(5)处的断言触发
void ReleaseAcquireDanger() {
	std::atomic<int> xd{ 0 }, yd{ 0 };
	std::atomic<int> zd;

	std::thread t1([&]() {
		xd.store(1, std::memory_order_release); //(1)
		yd.store(1, std::memory_order_release); //(2)
		});

	std::thread t2([&]() {
		yd.store(2, std::memory_order_release); //(3)
		});

	std::thread t3([&]() {
		while (!yd.load(std::memory_order_acquire)); //(4)
		assert(xd.load(std::memory_order_acquire) == 1); //(5)
		});

	t1.join();
	t2.join();
	t3.join();
}

/*
* 并不是只有在 acquire 操作读取到 release 操作写入的值时才能构成 synchronizes-with 关系. 
* 为了说这种情况, 我们需要引入 release sequence 这个概念.
*/

/*
* 针对一个原子变量 M 的 release 操作 A 完成后, 接下来 M 上可能还会有一连串的其他操作. 如果这一连串操作是由
*	同一线程上的写操作
*	任意线程上的 read-modify-write 操作
* 这两种构成的, 则称这一连串的操作为以release操作A为首的release sequence. 
* 这里的写操作和 read-modify-write 操作可以使用任意内存顺序.
* 如果一个 acquire 操作在同一个原子变量上读到了一个 release 操作写入的值, 
* 或者读到了以这个 release 操作为首的 release sequence 写入的值, 
* 那么这个 release 操作 “synchronizes-with” 这个 acquire 操作.
*/

/*
* t3要想退出首先flag要等于2，那么就要等到t2将flag设置为2，而flag设置为2又要等到t1将flag设置为1. 
* 所以我们捋一下顺序 2->3->4;
* t1中操作2是release操作，以2为开始，其他线程(t2)的读改写在release操作之后，
* 我们称之为release sequence， t3要读取release sequence写入的值，
* 所以我们称t1的release操作 “synchronizes with “ t3的 acquire 操作。
*/
void ReleaseSequence() {
	std::vector<int> data;
	std::atomic<int> flag{ 0 };

	std::thread t1([&]() {
		data.push_back(42); //(1)
		flag.store(1, std::memory_order_release); //(2)
		});

	std::thread t2([&]() {
		int expected = 1;
		while (!flag.compare_exchange_strong(expected, 2, std::memory_order_relaxed)) //(3)
			expected = 1;
		});

	std::thread t3([&]() {
		while (flag.load(std::memory_order_acquire) < 2); //(4)
		assert(data.at(0) == 42); //(5)
		});

	t1.join();
	t2.join();
	t3.join();
}

/*
* memory_order_consume 可以用于 load 操作. 使用 memory_order_consume 的 load 称为 consume 操作. 
* 如果一个 consume 操作在同一个原子变量上读到了一个 release 操作写入的值,
* 或以其为首的 release sequence 写入的值, 
* 则这个 release 操作 “dependency-ordered before” 这个 consume 操作.
*/

/*
* t2执行到(4)处时，需要等到ptr非空才能退出循环，这就依赖t1执行完(3)操作。
* 因此(3) “dependency-ordered before” (4), dependency等同于synchronizes ，所以(3) “inter-thread happens-before”. （4）
* 因为(1) “sequenced before” (3), 所以(1) “happens-before “ (4)
* 因为(4) “sequenced before” (5), 所以(1) “happens-before “ (5)
* 所以(5)处断言也不会触发。
* 因为(2) 和(3)不构成先行关系，所以(6)处断言可能触发.
*/
void ConsumeDependency() {
	std::atomic<std::string*> ptr;
	int data;

	std::thread t1([&]() {
		std::string* p = new std::string("Hello World"); //(1)
		data = 42; //(2)
		ptr.store(p, std::memory_order_release); //(3)
		});

	std::thread t2([&]() {
		std::string* p2;
		while (!(p2 = ptr.load(std::memory_order_consume))); //(4)
		assert(*p2 == "Hello World"); //(5)
		assert(data == 42); //(6)
		});

	t1.join();
	t2.join();
}

//单例模式改良
//利用智能指针解决释放问题
//原版：1处和4处存在线程安全问题
//new一个对象再赋值给变量时会存在多个指令顺序:
//两种情况：
//一、1、为对象allocate一块内存空间
//    2、调用construct构造对象
//    3、将构造的对象地址返回
//二、1 为对象allocate一块内存空间
//	  2 先将开辟的空间地址返回
//	  3 调用construct构造对象
/*
* 如果是第二种情况，在4处还未构造对象就将地址返回赋值给single，
* 而此时有线程运行至1处判断single不为空直接返回单例实例，
* 如果该线程调用这个单例的成员函数就会崩溃。
*/

class SingleAuto 
{
private:
	SingleAuto(){}
	SingleAuto(const SingleAuto&) = delete;
	SingleAuto& operator=(const SingleAuto&) = delete;
public:
	~SingleAuto() {
		std::cout << "single auto delete success " << std::endl;
	}

	static std::shared_ptr<SingleAuto> GetInst() {
		//1处
		if (single != nullptr) {
			return single;
		}
		//2处
		s_mutex.lock();
		//3处
		if (single != nullptr) { //这里有可能在1进行判断后，single为null，但是其他线程在之后创建了single)
			s_mutex.unlock();
			return single;
		}
		//4处
		single = std::shared_ptr<SingleAuto>(new SingleAuto); //这一步如果发生第二种情况，那么SingleAuto还没有被构造
		                                                      //single就获得了其地址，这时其他线程的1处就会判断成功
		s_mutex.unlock();
		return single;
	}
private:
	static std::shared_ptr<SingleAuto> single;
	static std::mutex s_mutex;
};

std::shared_ptr<SingleAuto> SingleAuto::single = nullptr;
std::mutex SingleAuto::s_mutex;

void TestSingle() {
	std::thread t1([]() {
		std::cout << "thread t1 singleton address is 0x: " << SingleAuto::GetInst() << std::endl;
		});
	std::thread t2([]() {
		std::cout << "thread t2 singleton address is 0x: " << SingleAuto::GetInst() << std::endl;
		});

	t2.join();
	t1.join();
}

//为解决上面1处和4处的问题，使用内存模型
class SingleMemoryModel
{
private:
	SingleMemoryModel(){}
	SingleMemoryModel(const SingleMemoryModel&) = delete;
	SingleMemoryModel& operator=(const SingleMemoryModel&) = delete;
public:
	~SingleMemoryModel()
	{
		std::cout << "single auto delete success " << std::endl;
	}
	static std::shared_ptr<SingleMemoryModel> GetInst()
	{
		// 1 处
		if (_b_init.load(std::memory_order_acquire))
		{
			return single;
		}
		// 2 处
		s_mutex.lock();
		// 3 处
		if (_b_init.load(std::memory_order_relaxed))
		{
			s_mutex.unlock();
			return single;
		}
		// 4处
		single = std::shared_ptr<SingleMemoryModel>(new SingleMemoryModel); 
		_b_init.store(true, std::memory_order_release); //这里添加内存模型，只要上一步对象没有完全创建成功
		                                                //那么_b_init就是false，1处的判断就不会成功；
		s_mutex.unlock();
		return single;
	}
private:
	static std::shared_ptr<SingleMemoryModel> single;
	static std::mutex s_mutex;
	static std::atomic<bool> _b_init;
};

std::shared_ptr<SingleMemoryModel> SingleMemoryModel::single = nullptr;
std::mutex SingleMemoryModel::s_mutex;
std::atomic<bool> SingleMemoryModel::_b_init = false;

void TestSingleMemory() {
	std::thread t1([]() {
		std::cout << "thread t1 singleton address is 0x: " << SingleMemoryModel::GetInst() << std::endl;
		});
	std::thread t2([]() {
		std::cout << "thread t2 singleton address is 0x: " << SingleMemoryModel::GetInst() << std::endl;
		});
	t2.join();
	t1.join();
}

int main() {

	//TestOrderSeqCst();
	//TestOrderRelaxed();
	//TestReleaseAcquire();
	//TestSingle();
	TestSingleMemory();

	return 0;
}