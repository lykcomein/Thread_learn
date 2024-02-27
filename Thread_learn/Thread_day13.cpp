#include <iostream>
#include <thread>
#include <atomic>
#include <cassert>
std::atomic<bool> x, y;
std::atomic<int> z;

void write_x() {
	x.store(true, std::memory_order_release); //1
}

void write_y() {
	y.store(true, std::memory_order_release);
}

void read_x_then_y() {
	while (!x.load(std::memory_order_acquire));
	if (y.load(std::memory_order_acquire)) {
		++z;
	}
}

void read_y_then_x() {
	while (!y.load(std::memory_order_acquire));
	if (x.load(std::memory_order_acquire)) {
		++z;
	}
}

void TestAR() {
	x = false;
	y = false;
	z = 0;
	/*
	* 对于a，b两个线程对x,y的修改，分别位于两个不同的线程，
	* 而c，d两个线程同样，也处于另外不同的两个线程，所以当他们读取
	* x,y的值时，相当于去两个不同的cpu中读取，而a，b作为两个线程，并不能保证
	* c，d读取x，y时并不能保证顺序(即使是release，require关系)
	*/

	/*
	* 即便是releas和acquire顺序也不能保证多个线程看到的一个变量的值是一致的，
	* 更不能保证看到的多个变量的值是一致的.
	*/
	std::thread a(write_x);
	std::thread b(write_y);
	std::thread c(read_x_then_y);
	std::thread d(read_y_then_x);

	a.join();
	b.join();
	c.join();
	d.join();

	assert(z.load() != 0); //5
	std::cout << "z value is " << z.load() << std::endl;
}

void write_x_then_y2() {
	x.store(true, std::memory_order_relaxed); //1
	y.store(true, std::memory_order_relaxed); //2
}

void read_y_then_x2() {
	while (!y.load(std::memory_order_relaxed)); //3
	if (x.load(std::memory_order_relaxed)) { //4
		++z;
	}
}

void TestRelaxed() {
	x = false;
	y = false;
	z = 0;
	std::thread a(write_x_then_y2);
	std::thread b(read_y_then_x2);
	a.join();
	b.join();
	assert(z.load() != 0); //5
}

void write_x_then_y3() {
	x.store(true, std::memory_order_relaxed); //1
	y.store(true, std::memory_order_release); //2
}

void read_y_then_x3() {
	while (!y.load(std::memory_order_acquire)); //3
	if (x.load(std::memory_order_relaxed)) {
		++z; 
	}
}

void write_x_then_y_fence() {
	x.store(true, std::memory_order_relaxed); //1
	std::atomic_thread_fence(std::memory_order_release); //2
	y.store(true, std::memory_order_relaxed); //3
}

void read_y_then_x_fence() {
	while (!y.load(std::memory_order_relaxed)); //4
	std::atomic_thread_fence(std::memory_order_acquire); //5
	if (x.load(std::memory_order_relaxed)) {
		++z;
	}
}

void TestFence() {
	x = false;
	y = false;
	z = 0;
	std::thread a(write_x_then_y_fence);
	std::thread b(read_y_then_x_fence);
	a.join();
	b.join();
	assert(z.load() != 0); //7
}

int main() {
	TestAR();

	return 0;
}