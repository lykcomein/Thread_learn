//asyncԴ��������Լ���������
#include <iostream>
#include <thread>
#include <memory>
#include <functional>
#include <future>

//����������һ�������͵ľֲ�����ʱ���ȵ����ƶ����죬���û���ƶ������ٵ��ÿ�������
//���Ȱ����ƶ�����ķ�ʽ���ؾֲ����������һ���ô����ǿ��Է���һЩֻ֧���ƶ����������

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

	//���ܽ�һ���̹߳���Ȩ�󶨸�һ���Ѿ����̵߳ı���������ᴥ��terminate���±���
	t1 = std::move(t2);
	t1.join();
	t2.join();
}


//������������BlockAsync()�� ����async��û���첽ִ�����񣬶��ǰ��������
//��Ϊasync����һ����ֵ���͵�future��������ֵ������ֵ��future��Ҫ����������Ϊ�䴦��һ���ֲ�������{}�С�
//��������ִ�е�}ʱ�ᴥ��future����������future����Ҫ��֤�������������ɣ�������Ҫ�ȴ��������future�ű�������
//����Ҳ�ͳ��˴��е�Ч���ˡ�
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

//������һ���������
//����������߳������DeadLock begin ����������ʱasync����һ���̣߳���ôlambda���ʽ���������std::async called ��.
//���������߳����޷������ɹ�����Ϊ���߳�û���ͷ����������߳��޷��ͷ�������Ϊ���߳�Ҫ�ȴ�asyncִ����
//����futures���ھֲ������򣬼�����������������Ҫ�ȴ�������ɣ�������Ҫ������������Զ��ɲ��ˣ�������������
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

//����ʵ����������
/*
* 1��func1 ��Ҫ�첽ִ��asyncFunc������
* 2��func2�����ռ�asyncFunc�������еĽ����ֻ�н����ȷ��ִ��
* 3��func1�����첽��������ִ�У�ִ����ֱ���˳����õȵ�asyncFunc������
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
//���ﱣ֤func1��func2ʹ�õ���future�����ã�����func1�ڲ�����Ϊ����async������
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