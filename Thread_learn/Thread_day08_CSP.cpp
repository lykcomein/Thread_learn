//Actor和CSP设计模式

/*
* actor通过消息传递的方式与外界通信。消息传递是异步的。
* 每个actor都有一个邮箱，该邮箱接收并缓存其他actor发过来的消息，
* actor一次只能同步处理一个消息，处理消息过程中，除了可以接收消息，不能做任何其他操作。
* 每一个类独立在一个线程里称作Actor，Actor之间通过队列通信，比如Actor1 发消息给Actor2， 
* Actor2 发消息给Actor1都是投递到对方的队列中。
*/

/*
* CSP 是 Communicating Sequential Process 的简称，中文可以叫做通信顺序进程，
* 是一种并发编程模型，是一个很强大的并发数据模型，
* 是上个世纪七十年代提出的，用于描述两个独立的并发实体通过共享的 通讯 channel(管道)进行通信的并发模型。
* CSP中channel是第一类对象，它不关注发送消息的实体，而关注与发送消息时使 用的channel。
* CSP将消息投递给channel，至于谁从channel中取数据，发送的一方是不关注的。
*/

//用C++实现一个channel，采用csp模式解耦合，实现生产者和消费者问题

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class Channel {
private:
	std::queue<T> queue_;
	std::mutex mtx_;
	std::condition_variable cv_producer_;
	std::condition_variable cv_consumer_;
	size_t capacity_;
	bool closed_ = false;

public:
	Channel(size_t capacity = 0):capacity_(capacity){}
	bool send(T value) {
		std::unique_lock<std::mutex> lock(mtx_);
		cv_producer_.wait(lock, [this]() {
			//对于无缓冲的channel，应该等待直到有消费者准备好
			return (capacity_ == 0 && queue_.empty()) || queue_.size() < capacity_ || closed_;
			});

		if (closed_) {
			return false;
		}

		queue_.push(value);
		cv_consumer_.notify_one();
		return true;
	}

	bool receive(T& value) {
		std::unique_lock<std::mutex> lock(mtx_);
		cv_consumer_.wait(lock, [this]() {
			return !queue_.empty() || closed_;
			});
		if (closed_ && queue_.empty()) {
			return false;
		}

		value = queue_.front();
		queue_.pop();
		cv_producer_.notify_one();
		return true;
	}

	void close() {
		std::unique_lock<std::mutex> lock(mtx_);
		closed_ = true;
		cv_producer_.notify_all();
		cv_consumer_.notify_all();
	}

};

//示例使用
int main() {
	Channel<int> ch(10); //无缓冲的channel

	std::thread producer([&]() {
		for (int i = 0; i < 5; i++) {
			ch.send(i);
			std::cout << "Sent: " << i << std::endl;
		}
		ch.close();
		});

	std::thread consumer([&]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); //故意延迟消费者开始消费
		int val;
		while (ch.receive(val)) {
			std::cout << "Received: " << val << std::endl;
		}
		});

	producer.join();
	consumer.join();
	return 0;
}
