//Actor��CSP���ģʽ

/*
* actorͨ����Ϣ���ݵķ�ʽ�����ͨ�š���Ϣ�������첽�ġ�
* ÿ��actor����һ�����䣬��������ղ���������actor����������Ϣ��
* actorһ��ֻ��ͬ������һ����Ϣ��������Ϣ�����У����˿��Խ�����Ϣ���������κ�����������
* ÿһ���������һ���߳������Actor��Actor֮��ͨ������ͨ�ţ�����Actor1 ����Ϣ��Actor2�� 
* Actor2 ����Ϣ��Actor1����Ͷ�ݵ��Է��Ķ����С�
*/

/*
* CSP �� Communicating Sequential Process �ļ�ƣ����Ŀ��Խ���ͨ��˳����̣�
* ��һ�ֲ������ģ�ͣ���һ����ǿ��Ĳ�������ģ�ͣ�
* ���ϸ�������ʮ�������ģ������������������Ĳ���ʵ��ͨ������� ͨѶ channel(�ܵ�)����ͨ�ŵĲ���ģ�͡�
* CSP��channel�ǵ�һ�����������ע������Ϣ��ʵ�壬����ע�뷢����Ϣʱʹ �õ�channel��
* CSP����ϢͶ�ݸ�channel������˭��channel��ȡ���ݣ����͵�һ���ǲ���ע�ġ�
*/

//��C++ʵ��һ��channel������cspģʽ����ϣ�ʵ�������ߺ�����������

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
			//�����޻����channel��Ӧ�õȴ�ֱ����������׼����
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

//ʾ��ʹ��
int main() {
	Channel<int> ch(10); //�޻����channel

	std::thread producer([&]() {
		for (int i = 0; i < 5; i++) {
			ch.send(i);
			std::cout << "Sent: " << i << std::endl;
		}
		ch.close();
		});

	std::thread consumer([&]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); //�����ӳ������߿�ʼ����
		int val;
		while (ch.receive(val)) {
			std::cout << "Received: " << val << std::endl;
		}
		});

	producer.join();
	consumer.join();
	return 0;
}
