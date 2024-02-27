//�ڴ�˳��ʵ���ڴ�ģ��

/*
* ����˳��ģ�ͣ�
* Sequencial consistent ordering��ʵ��ͬ��, �ұ�֤ȫ��˳��һ�� (single total order) ��ģ��. ��һ������ǿ��ģ��, Ҳ��Ĭ�ϵ�˳��ģ��.
* Acquire-release ordering��ʵ��ͬ��, ������֤��֤ȫ��˳��һ�µ�ģ��.
* Relaxed ordering������ʵ��ͬ����ֻ�ܱ�֤ԭ���Ե�ģ��
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
* ����Ĵ���x��y���õ���memory_order_seq_cst, ���Ե��߳�t2ִ�е�3�����˳�ѭ��ʱ���ǿ��Զ϶�yΪtrue��
* ��Ϊ��ȫ��һ����˳�������߳�t1�Ѿ�ִ����2����y����Ϊtrue����ô�߳�t1Ҳһ��ִ����1�����벢��t2�ɼ���
* ���Ե�t2ִ����4��ʱxΪtrue����ô��ִ��z++��֤z��Ϊ�㣬�Ӷ����ᴥ�����ԡ�
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
* ��ԭ�ӱ����� load ����ʹ�� memory_order_acquire �ڴ�˳��. ���Ϊ acquire ����.
* ��ԭ�ӱ����� store ����ʹ�� memory_order_release �ڴ�˳��. ���Ϊ release ����.
* read-modify-write �������� (load) ��д (store),
* ������ʹ�� memory_order_acquire, memory_order_release �� memory_order_acq_rel:
*/

/*
* ry.storeʹ�õ���std::memory_order_release, ry.loadʹ�õ���std::memory_order_acquire.
* t1ִ�е�2��ry ����Ϊtrue, ��Ϊʹ����Acquire-release ˳��
* ���� t2 ִ�е�3ʱ��ȡryΪtrue�� ���2��3 ���Թ���ͬ����ϵ��
* ����Ϊ���߳�t1�� 1 sequence before 2,����1 happens-before 3.
* ��Ϊ���߳�t2�� 3 sequence before 4. ���� 1 happens-before 4.
* ���Զ϶�4 ���ᴥ�����ԡ�
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

//t3��ydΪtrueʱ�Ż��˳�����������������
//һ����yd��1��һ����yd��2��
//��yd��2ʱ�����ܳ���t2(3)ִ�н�����(1)��û��ִ�н����������
//��ʱ(5)���Ķ��Դ���
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
* ������ֻ���� acquire ������ȡ�� release ����д���ֵʱ���ܹ��� synchronizes-with ��ϵ. 
* Ϊ��˵�������, ������Ҫ���� release sequence �������.
*/

/*
* ���һ��ԭ�ӱ��� M �� release ���� A ��ɺ�, ������ M �Ͽ��ܻ�����һ��������������. �����һ������������
*	ͬһ�߳��ϵ�д����
*	�����߳��ϵ� read-modify-write ����
* �����ֹ��ɵ�, �����һ�����Ĳ���Ϊ��release����AΪ�׵�release sequence. 
* �����д������ read-modify-write ��������ʹ�������ڴ�˳��.
* ���һ�� acquire ������ͬһ��ԭ�ӱ����϶�����һ�� release ����д���ֵ, 
* ���߶���������� release ����Ϊ�׵� release sequence д���ֵ, 
* ��ô��� release ���� ��synchronizes-with�� ��� acquire ����.
*/

/*
* t3Ҫ���˳�����flagҪ����2����ô��Ҫ�ȵ�t2��flag����Ϊ2����flag����Ϊ2��Ҫ�ȵ�t1��flag����Ϊ1. 
* ����������һ��˳�� 2->3->4;
* t1�в���2��release��������2Ϊ��ʼ�������߳�(t2)�Ķ���д��release����֮��
* ���ǳ�֮Ϊrelease sequence�� t3Ҫ��ȡrelease sequenceд���ֵ��
* �������ǳ�t1��release���� ��synchronizes with �� t3�� acquire ������
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
* memory_order_consume �������� load ����. ʹ�� memory_order_consume �� load ��Ϊ consume ����. 
* ���һ�� consume ������ͬһ��ԭ�ӱ����϶�����һ�� release ����д���ֵ,
* ������Ϊ�׵� release sequence д���ֵ, 
* ����� release ���� ��dependency-ordered before�� ��� consume ����.
*/

/*
* t2ִ�е�(4)��ʱ����Ҫ�ȵ�ptr�ǿղ����˳�ѭ�����������t1ִ����(3)������
* ���(3) ��dependency-ordered before�� (4), dependency��ͬ��synchronizes ������(3) ��inter-thread happens-before��. ��4��
* ��Ϊ(1) ��sequenced before�� (3), ����(1) ��happens-before �� (4)
* ��Ϊ(4) ��sequenced before�� (5), ����(1) ��happens-before �� (5)
* ����(5)������Ҳ���ᴥ����
* ��Ϊ(2) ��(3)���������й�ϵ������(6)�����Կ��ܴ���.
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

//����ģʽ����
//��������ָ�����ͷ�����
//ԭ�棺1����4�������̰߳�ȫ����
//newһ�������ٸ�ֵ������ʱ����ڶ��ָ��˳��:
//���������
//һ��1��Ϊ����allocateһ���ڴ�ռ�
//    2������construct�������
//    3��������Ķ����ַ����
//����1 Ϊ����allocateһ���ڴ�ռ�
//	  2 �Ƚ����ٵĿռ��ַ����
//	  3 ����construct�������
/*
* ����ǵڶ����������4����δ�������ͽ���ַ���ظ�ֵ��single��
* ����ʱ���߳�������1���ж�single��Ϊ��ֱ�ӷ��ص���ʵ����
* ������̵߳�����������ĳ�Ա�����ͻ������
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
		//1��
		if (single != nullptr) {
			return single;
		}
		//2��
		s_mutex.lock();
		//3��
		if (single != nullptr) { //�����п�����1�����жϺ�singleΪnull�����������߳���֮�󴴽���single)
			s_mutex.unlock();
			return single;
		}
		//4��
		single = std::shared_ptr<SingleAuto>(new SingleAuto); //��һ����������ڶ����������ôSingleAuto��û�б�����
		                                                      //single�ͻ�������ַ����ʱ�����̵߳�1���ͻ��жϳɹ�
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

//Ϊ�������1����4�������⣬ʹ���ڴ�ģ��
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
		// 1 ��
		if (_b_init.load(std::memory_order_acquire))
		{
			return single;
		}
		// 2 ��
		s_mutex.lock();
		// 3 ��
		if (_b_init.load(std::memory_order_relaxed))
		{
			s_mutex.unlock();
			return single;
		}
		// 4��
		single = std::shared_ptr<SingleMemoryModel>(new SingleMemoryModel); 
		_b_init.store(true, std::memory_order_release); //��������ڴ�ģ�ͣ�ֻҪ��һ������û����ȫ�����ɹ�
		                                                //��ô_b_init����false��1�����жϾͲ���ɹ���
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