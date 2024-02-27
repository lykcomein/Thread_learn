//�����������Ʋ���ͬ��
#include <iostream>
#include <mutex>

int num = 1;
std::mutex mtx_num;
std::condition_variable cvA;
std::condition_variable cvB;

//�߳�t1���1��t2���2���������
//����PoorImpleman����ʵ�ֽ����ӡ�Ĺ��ܣ�
//���������Ϣ����Ĳ���ʱ����Ϊ�߳�AҪѭ�����numֵ��
//���num��Ϊ1�����߳�A��˯���ˣ����߳�A˯�����ʱ��ܿ���B�Ѿ��������ӡ�ˣ�
//��ʱA����˯�ߣ��Ƕ���Դ���˷ѣ�Ҳ�������ѵĴ���ʱ����
void PoorImplemention() {
	std::thread t1([]() {
		for (;;) {
			//t1���������num==1���������Ȼ�������sleepһ�᣻
			//����ֱ�ӽ�����Ȼ��sleepһ��
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
			//t2��t1ͬ�����2��
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

//ʹ����������ʵ��������߼�
//cvA�ȴ�num==1ʱ���ͻ�����������Ϊ1�������(cvA.wait);
//ֱ��cvB֪ͨA����A����
//t2ͬ��
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

//ʹ����������ʵ��һ���̰߳�ȫ����
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