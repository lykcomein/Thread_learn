//����ģʽ�ݱ�
#include <iostream>
#include <mutex>
#include <thread>
#include <memory>

//���ڵ���ģʽ
//C++11 ��ǰ�÷�ʽ���ڷ��գ��ڶ���̳߳�ʼ��ʱ���ڿ��ٶ��ʵ�����
//C++ 11�Ժ�󲿷ֵĵ������ع鵽���ģʽ��

class Single2 {
private:
	Single2(){}
	Single2(const Single2&) = delete;
	Single2& operator=(const Single2&) = delete;
public:
	static Single2& GetInst() {
		static Single2 single;
		return single;
	}
};

void test_single2() {
	//���߳�����¿��ܴ�������
	std::cout << "this thread id: " << std::this_thread::get_id() << std::endl;
	std::cout << "s1 addr is " << &Single2::GetInst() << std::endl;
	std::cout << "s2 addr is " << &Single2::GetInst() << std::endl;
}
void thread_func_s1(int i) {
	std::cout << "this is thread " << i << std::endl;
	std::cout << "inst is " << &Single2::GetInst() << std::endl;
}

void test_single2_thread() {
	//std::thread t1(test_single2);
	//std::thread t2(test_single2);
	//t1.join();
	//t2.join();
	for (int i = 0; i < 6; i++) {
		std::thread tid(thread_func_s1, i);
		tid.join();
	}
}

/*
*�����汾�ĵ���ģʽ��C++11 ��ǰ���ڶ��̲߳���ȫ����������������ܻ��ʼ�������̬������
*/

//����ʽ
class Single2Hungry {
private:
	Single2Hungry(){}
	Single2Hungry(const Single2Hungry&) = delete;
	Single2Hungry& operator=(const Single2Hungry&) = delete;
public:
	static Single2Hungry* GetInst() {
		if (single == nullptr) {
			single = new Single2Hungry();
		}
		return single;
	}
private:
	static Single2Hungry* single;
};

//����ʽ��ʼ��
Single2Hungry* Single2Hungry::single = Single2Hungry::GetInst();
void thread_func_s2(int i) {
	std::cout << "this is thread " << i << std::endl;
	std::cout << "inst is " << Single2Hungry::GetInst() << std::endl;
}
void test_single2hungry() {
	std::cout << "s1 addr is " << Single2Hungry::GetInst() << std::endl;
	std::cout << "s2 addr is " << Single2Hungry::GetInst() << std::endl;
	for (int i = 0; i < 3; i++) {
		std::thread tid(thread_func_s2, i);
		tid.join();
	}
}

//����û�����ʹ������ʽ����Ҫ����
//���ǻ���ָ�����ͬ�������⣺�����ͷŻ�֪���ĸ�ָ���ͷţ�

class SinglePointer {
private:
	SinglePointer(){}
	SinglePointer(const SinglePointer&) = delete;
	SinglePointer& operator=(const SinglePointer&) = delete;

public:
	static SinglePointer* GetInst() {
		if (single != nullptr) {
			return single;
		}
		s_mutex.lock();
		if (single != nullptr) {
			s_mutex.unlock();
			return single;
		}
		single = new SinglePointer();
		s_mutex.unlock();
		return single;
	}

private:
	static SinglePointer* single;
	static std::mutex s_mutex;
};

//��������
SinglePointer* SinglePointer::single = nullptr;
std::mutex SinglePointer::s_mutex;
void thread_func_lazy(int i) {
	std::cout << "this is lazy thread " << i << std::endl;
	std::cout << "inst is " << SinglePointer::GetInst() << std::endl;
}
void test_singlelazy() {
	for (int i = 0; i < 3; i++) {
		std::thread tid(thread_func_lazy, i);
		tid.join();
	}
	//��ʱ�ͷ�new�Ķ�������ڴ�й©
}

//��������ָ������Զ�����
class SingleAuto {
private:
	SingleAuto(){}
	SingleAuto(const SingleAuto&) = delete;
	SingleAuto& operator=(const SingleAuto&) = delete;

public:
	~SingleAuto() {
		std::cout << "single auto delete success " << std::endl;
	}
	static std::shared_ptr<SingleAuto> GetInst() {
		if (single != nullptr)
		{
			return single;
		}
		s_mutex.lock();
		if (single != nullptr)
		{
			s_mutex.unlock();
			return single;
		}
		single = std::shared_ptr<SingleAuto>(new SingleAuto);
		s_mutex.unlock();
		return single;
	}

private:
	static std::shared_ptr<SingleAuto> single;
	static std::mutex s_mutex;
};

std::shared_ptr<SingleAuto> SingleAuto::single = nullptr;
std::mutex SingleAuto::s_mutex;
void test_singleauto() {
	auto sp1 = SingleAuto::GetInst();
	auto sp2 = SingleAuto::GetInst();
	std::cout << "sp1 is " << sp1 << std::endl;
	std::cout << "sp2 is " << sp2 << std::endl;
	//��ʱ���������������ֶ�ɾ����ָ�룬��ɱ���
	//delete sp1.get();
}

//Ϊ�˹���û��ֶ��ͷ��ڴ棬�����ṩһ���������æ�����ڴ�
//�������������������дΪ˽��

class SingleAutoSafe;
class SafeDeletor {
public:
	void operator()(SingleAutoSafe* sf) {
		std::cout << "this is safe deleter operator()" << std::endl;
		delete sf;
	}
};

class SingleAutoSafe {
private:
	SingleAutoSafe(){}
	~SingleAutoSafe() {
		std::cout << "this is single auto safe deletor" << std::endl;
	}
	SingleAutoSafe(const SingleAutoSafe&) = delete;
	SingleAutoSafe& operator=(const SingleAutoSafe&) = delete;
	//������Ԫ�࣬ͨ����Ԫ����ø������������
	friend class SafeDeletor;

public:
	static std::shared_ptr<SingleAutoSafe> GetInst() {
		//1��
		if (single != nullptr)
		{
			return single;
		}
		s_mutex.lock();
		//2��
		if (single != nullptr)
		{
			s_mutex.unlock();
			return single;
		}
		//����ָ��ɾ����
		//3 ��
		single = std::shared_ptr<SingleAutoSafe>(new SingleAutoSafe, SafeDeletor());
		//Ҳ����ָ��ɾ������
		//single = std::shared_ptr<SingleAutoSafe>(new SingleAutoSafe, SafeDelFunc);
		s_mutex.unlock();
		return single;
	}
private:
	static std::shared_ptr<SingleAutoSafe> single;
	static std::mutex s_mutex;
};

std::shared_ptr<SingleAutoSafe>
SingleAutoSafe::single = nullptr;
std::mutex SingleAutoSafe::s_mutex;

//��������Ĵ������Σ�գ���������ʽ��ʹ�÷�ʽ��������̵߳��õ���ʱ����һ���̼߳�������3�����߼���
//�������߳��е���1�����ж�ָ��ǿ���������ʼ��ֱ��ʹ�õ������ڴ��������⡣
//��Ҫԭ������SingleAutoSafe * temp =  new SingleAutoSafe()  �������������������ɵ�
//1 ����allocate�����ڴ�
//2 ����constructִ��SingleAutoSafe�Ĺ��캯��
//3 ���ø�ֵ��������ַ��ֵ��temp
//����ʵ��2��3�Ĳ�����ܵߵ��������п�����һЩ��������ͨ���Ż���1��3��2�ĵ���˳��
//�����߳�ȡ����ָ����Ƿǿգ���û���ļ����ù��캯���ͽ����ⲿʹ����ɲ���Ԥ֪����
//Ϊ���������⣬C++11 �Ƴ���std::call_once������֤����߳�ִֻ��һ��

class SingletonOnce {
private:
	SingletonOnce() = default;
	SingletonOnce(const SingletonOnce&) = delete;
	SingletonOnce& operator = (const SingletonOnce& st) = delete;
	static std::shared_ptr<SingletonOnce> _instance;

public:
	static std::shared_ptr<SingletonOnce> GetInstance() {
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]() {
			_instance = std::shared_ptr<SingletonOnce>(new SingletonOnce);
			});
		return _instance;
	}
	void PrintAddress() {
		std::cout << _instance.get() << std::endl;
	}

	~SingletonOnce() {
		std::cout << "this is singleton destruct" << std::endl;
	}
};

std::shared_ptr<SingletonOnce>
SingletonOnce::_instance = nullptr;

void TestSingle() {
	std::thread t1([]() {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		SingletonOnce::GetInstance()->PrintAddress();
		});
	std::thread t2([]() {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		SingletonOnce::GetInstance()->PrintAddress();
		});

	t1.join();
	t2.join();
}

//Ϊ���õ�������ͨ�ã���������ģ����
template<typename T>
class Singleton {
protected:
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>& st) = delete;
	static std::shared_ptr<T> _instance;

public:
	static std::shared_ptr<T> GetInstance() {
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]() {
			_instance = std::shared_ptr<T>(new T);
			});
		return _instance;
	}
	void PrintAddress() {
		std::cout << _instance.get() << std::endl;
	}
	~Singleton() {
		std::cout << "this is singleton destruct" << std::endl;
	}
};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

//��ʹ�õ����࣬���Լ̳������ģ��
class LogicSystem :public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem() {}
private:
	LogicSystem() {}
};

int main() {
	//test_single2_thread();
	//test_single2hungry();
	//test_singlelazy();
	TestSingle();
	return 0;
}