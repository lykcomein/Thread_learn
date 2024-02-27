//内存模型，原子类型数据
/*
* 标准原子类型定义于<atomic>中，可以通过atomic<>定义一些原子类型的变量
* 如：atomic<bool>, atomic<int>;
* 
* std::atomic_flag:原子类型，所有atomic<>的原子类型都可以基于它实现
* std::atomic_flag的test_and_set成员函数是一个原子操作，
* 它会查验std::atomic_flag当前状态是否被设置过；
* 1、如果没有被设置过：将std::atomic_flag当前的状态设置为true;并返回false；
* 2、如果被设置过，直接返回true；
* 
*/

/*
* 对于原子类型上的每一种操作，都可以提供额外的参数，从枚举类std::memory_order取值，用于设定所需的内存次序语义
* 枚举类std::memory_order具有6个可能的值，
*
* 包括std::memory_order_relaxed、std:: memory_order_acquire、std::memory_order_consume、
* std::memory_order_acq_rel、std::memory_order_release和 std::memory_order_seq_cst。
* 
*/
/*
* 存储（store）操作，可选用的内存次序有:
* std::memory_order_relaxed、
* std::memory_order_release、
* 或std::memory_order_seq_cst。
*
* 载入（load）操作，可选用的内存次序有:
* std::memory_order_relaxed、
* std::memory_order_consume、
* std::memory_order_acquire、
* 或std::memory_order_seq_cst。
*
* “读 - 改 - 写”（read - modify - write）操作，可选用的内存次序有:
* std::memory_order_relaxed、
* std::memory_order_consume、
* std::memory_order_acquire、
* std::memory_order_release、
* std::memory_order_acq_rel
* 或std::memory_order_seq_cst。
*/

#include<iostream>
#include <atomic>
#include <thread>
#include <cassert>
#include <vector>

//实现自旋锁
class SpinLock {
public:
	void lock() {
		while (flag.test_and_set(std::memory_order_acquire)); //自选等待，直到成功获取到锁()这里使用memory_order_acquire次序
	}

	void unlock() {
		flag.clear(std::memory_order_release); //释放锁(使用memory_order_release次序)
	}

private:
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

void TestSpinLock() {
	SpinLock spinlock;
	std::thread t1([&spinlock]() {
		spinlock.lock();
		for (int i = 0; i < 3; i++) {
			std::cout << "*";
		}
		std::cout << std::endl;
		spinlock.unlock();
		});

	std::thread t2([&spinlock]() {
		spinlock.lock();
		for (int i = 0; i < 3; i++) {
			std::cout << "?";
		}
		std::cout << std::endl;
		spinlock.unlock();
		});
	t1.join();
	t2.join();
}
//线程t1调用test_and_set后，返回false，结束循环，（这里成功获取到了锁)
//然后线程t2调用test_and_set后返回true，处于循环中，直到t1调用unlock，进行clear操作
//此时t2调用test_and_set会返回false，退出循环，执行后续操作

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
	x.store(true, std::memory_order_relaxed); //1
	y.store(true, std::memory_order_relaxed); //2
}

void read_y_then_x() {
	while (!y.load(std::memory_order_relaxed)) { //3
		std::cout << "y load false" << std::endl;
	}

	if (x.load(std::memory_order_relaxed)) {//4
		++z;
	}
}
//从两个角度分析
/*
* 1 从cpu架构分析
	假设线程t1运行在CPU1上，t2运行在CPU3上，那么t1对x和y的操作，t2是看不到的。
	比如当线程t1运行至1处将x设置为true，t1运行至2处将y设置为true。
	这些操作仅在CPU1的store buffer中，还未放入cache和memory中，CPU2自然不知道。
	如果CPU1先将y放入memory，那么CPU2就会读取y的值为true。
	那么t2就会运行至3处从while循环退出，进而运行至4处，此时CPU1还未将x的值写入memory，
	t2读取的x值为false，进而线程t2运行结束，
	然后CPU1将x写入true, t1结束运行，最后主线程运行至5处，因为z为0,所以触发断言。
* 2 从宽松内存序分析
	因为memory_order_relaxed是宽松的内存序列，它只保证操作的原子性，
	并不能保证多个变量之间的顺序性，也不能保证同一个变量在不同线程之间的可见顺序。
	比如t1可能先运行2处代码再运行1处代码，因为我们的代码会被编排成指令执行，
	编译器在不破坏语义的情况下(2处和1处代码无耦合，可调整顺序)，2可能先于1执行。
	如果这样，t2运行至3处退出while循环，继续运行4处，此时t1还未执行1初代码，
	则t2运行4处条件不成立不会对z做增加，t2结束。这样也会导致z为0引发断言。
*/

void TestOrderRelaxed() {
	std::thread t1(write_x_then_y);
	std::thread t2(read_y_then_x);
	t1.join();
	t2.join();
	assert(z.load() != 0);
}

void TestOrderRelaxed2() {
	std::atomic<int> a{ 0 };
	std::vector<int> v3, v4;
	std::thread t1([&a]() {
		for (int i = 0; i < 10; i += 2) {
			a.store(i, std::memory_order_relaxed);
		}
		});

	std::thread t2([&a]() {
		for (int i = 1; i < 10; i += 2) {
			a.store(i, std::memory_order_relaxed);
		}
		});

	std::thread t3([&v3, &a]() {
		for (int i = 0; i < 10; ++i) {
			v3.push_back(a.load(std::memory_order_relaxed));
		}
		});

	std::thread t4([&v4, &a]() {
		for (int i = 0; i < 10; ++i) {
			v4.push_back(a.load(std::memory_order_relaxed));
		}
		});

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	for (int i : v3) {
		std::cout << i << " ";
	}
	std::cout << std::endl;

	for (int i : v4) {
		std::cout << i << " ";
	}
	std::cout << std::endl;
}
//多个线程仅操作了a变量，通过memory_order_relaxed的方式仅能保证对a的操作是原子的(同一时刻仅有一个线程写a的值，但是可能多个线程读取a的值)。


int main() {


	//TestSpinLock();
	TestOrderRelaxed();
	//TestOrderRelaxed2();

	return 0;


}