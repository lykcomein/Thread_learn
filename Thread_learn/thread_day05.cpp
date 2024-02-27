//条件变量控制并发同步
#include <iostream>
#include <mutex>

int num = 1;
std::mutex mtx_num;
std::condition_variable cvA;
std::condition_variable cvB;

//线程t1输出1，t2输出2；交替输出
//下面PoorImpleman可以实现交替打印的功能，
//但会造成消息处理的不及时，因为线程A要循环检测num值，
//如果num不为1，则线程A就睡眠了，在线程A睡眠这段时间很可能B已经处理完打印了，
//此时A还在睡眠，是对资源的浪费，也错过了最佳的处理时机。
void PoorImplemention() {
	std::thread t1([]() {
		for (;;) {
			//t1加锁后，如果num==1，则输出，然后解锁，sleep一会；
			//否则，直接解锁，然后sleep一会
			{
				std::lock_guard<std::mutex> lock(mtx_num);
				if (num == 1) {
					std::cout << "thread A print 1....." << std::endl;
					num++;
					continue;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	});

	std::thread t2([]() {
		for (;;) {
			//t2与t1同理，输出2；
			{
				std::lock_guard<std::mutex>lock(mtx_num);
				if (num == 2) {
					std::cout << "thread B print 2....." << std::endl;
					num--;
					continue;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		});

	t1.join();
	t2.join();
}

//使用条件变量实现上面的逻辑
//cvA等待num==1时，就会输出；如果不为1，则挂起(cvA.wait);
//直到cvB通知A，将A唤醒
//t2同理
void ResponableImplemention() {
	std::thread t1([]() {
		for (;;) {
			std::unique_lock<std::mutex> lock(mtx_num);
			cvA.wait(lock, []() {
				return num == 1;
				});
			num++;
			std::cout << "thread A print 1......." << std::endl;
			cvB.notify_one();
		}
		});

	std::thread t2([]() {
		for (;;) {
			std::unique_lock<std::mutex> lock(mtx_num);
			cvB.wait(lock, []() {
				return num == 2;
				});
			num--;
			std::cout << "thread B print 2......" << std::endl;
			cvA.notify_one();
		}
		});
	t1.join();
	t2.join();
}

//使用条件变量实现一个线程安全队列
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue {
private:
	std::mutex mut;
	std::queue<T> data_queue;
	std::condition_variable data_cond;
public:
	void push(T new_value) {
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(new_value);
		data_cond.notify_one();
	}
	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this] {
			return !data_queue.empty();
			});
		value = data_queue.front();
		data_queue.pop();
	}

	std::shared_ptr<T> wait_and_pop() {
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this] {
			return !data_queue.empty();
			});
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}
	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty()) {
			return false;
		}
		value = data_queue.front();
		data_queue.pop();
		return true;
	}
	std::shared_ptr<T> try_pop() {
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty()) {
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}
	bool empty()const {
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}
};

void test_safe_que() {
	threadsafe_queue<int> safe_que;
	std::mutex mtx_print;
	std::thread producer(
		[&]() {
			for (int i = 0; ; i++) {
				safe_que.push(i);
				{
					std::lock_guard<std::mutex> printlk(mtx_print);
					std::cout << "producer push data is " << i << std::endl;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
	);

	std::thread consumer1(
		[&]() {
			for (;;) {
				auto data = safe_que.wait_and_pop();
				{
					std::lock_guard<std::mutex> printlk(mtx_print);
					std::cout << "consumer1 wait and pop data is " << *data << std::endl;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}
	);

	std::thread consumer2(
		[&]() {
			for (;;) {
				auto data = safe_que.wait_and_pop();
				{
					std::lock_guard<std::mutex> printlk(mtx_print);
					std::cout << "consumer2 wait and pop data is " << *data << std::endl;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}
	);

	producer.join();
	consumer1.join();
	consumer2.join();
}

int main() {

	//PoorImplemention();
	//ResponableImplemention();
	test_safe_que();
	return 0;
}