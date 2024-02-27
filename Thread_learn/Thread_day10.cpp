//�ڴ�ģ�ͣ�ԭ����������
/*
* ��׼ԭ�����Ͷ�����<atomic>�У�����ͨ��atomic<>����һЩԭ�����͵ı���
* �磺atomic<bool>, atomic<int>;
* 
* std::atomic_flag:ԭ�����ͣ�����atomic<>��ԭ�����Ͷ����Ի�����ʵ��
* std::atomic_flag��test_and_set��Ա������һ��ԭ�Ӳ�����
* �������std::atomic_flag��ǰ״̬�Ƿ����ù���
* 1�����û�б����ù�����std::atomic_flag��ǰ��״̬����Ϊtrue;������false��
* 2����������ù���ֱ�ӷ���true��
* 
*/

/*
* ����ԭ�������ϵ�ÿһ�ֲ������������ṩ����Ĳ�������ö����std::memory_orderȡֵ�������趨������ڴ��������
* ö����std::memory_order����6�����ܵ�ֵ��
*
* ����std::memory_order_relaxed��std:: memory_order_acquire��std::memory_order_consume��
* std::memory_order_acq_rel��std::memory_order_release�� std::memory_order_seq_cst��
* 
*/
/*
* �洢��store����������ѡ�õ��ڴ������:
* std::memory_order_relaxed��
* std::memory_order_release��
* ��std::memory_order_seq_cst��
*
* ���루load����������ѡ�õ��ڴ������:
* std::memory_order_relaxed��
* std::memory_order_consume��
* std::memory_order_acquire��
* ��std::memory_order_seq_cst��
*
* ���� - �� - д����read - modify - write����������ѡ�õ��ڴ������:
* std::memory_order_relaxed��
* std::memory_order_consume��
* std::memory_order_acquire��
* std::memory_order_release��
* std::memory_order_acq_rel
* ��std::memory_order_seq_cst��
*/

#include<iostream>
#include <atomic>
#include <thread>
#include <cassert>
#include <vector>

//ʵ��������
class SpinLock {
public:
	void lock() {
		while (flag.test_and_set(std::memory_order_acquire)); //��ѡ�ȴ���ֱ���ɹ���ȡ����()����ʹ��memory_order_acquire����
	}

	void unlock() {
		flag.clear(std::memory_order_release); //�ͷ���(ʹ��memory_order_release����)
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
//�߳�t1����test_and_set�󣬷���false������ѭ����������ɹ���ȡ������)
//Ȼ���߳�t2����test_and_set�󷵻�true������ѭ���У�ֱ��t1����unlock������clear����
//��ʱt2����test_and_set�᷵��false���˳�ѭ����ִ�к�������

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
//�������Ƕȷ���
/*
* 1 ��cpu�ܹ�����
	�����߳�t1������CPU1�ϣ�t2������CPU3�ϣ���ôt1��x��y�Ĳ�����t2�ǿ������ġ�
	���統�߳�t1������1����x����Ϊtrue��t1������2����y����Ϊtrue��
	��Щ��������CPU1��store buffer�У���δ����cache��memory�У�CPU2��Ȼ��֪����
	���CPU1�Ƚ�y����memory����ôCPU2�ͻ��ȡy��ֵΪtrue��
	��ôt2�ͻ�������3����whileѭ���˳�������������4������ʱCPU1��δ��x��ֵд��memory��
	t2��ȡ��xֵΪfalse�������߳�t2���н�����
	Ȼ��CPU1��xд��true, t1�������У�������߳�������5������ΪzΪ0,���Դ������ԡ�
* 2 �ӿ����ڴ������
	��Ϊmemory_order_relaxed�ǿ��ɵ��ڴ����У���ֻ��֤������ԭ���ԣ�
	�����ܱ�֤�������֮���˳���ԣ�Ҳ���ܱ�֤ͬһ�������ڲ�ͬ�߳�֮��Ŀɼ�˳��
	����t1����������2������������1�����룬��Ϊ���ǵĴ���ᱻ���ų�ָ��ִ�У�
	�������ڲ��ƻ�����������(2����1����������ϣ��ɵ���˳��)��2��������1ִ�С�
	���������t2������3���˳�whileѭ������������4������ʱt1��δִ��1�����룬
	��t2����4�����������������z�����ӣ�t2����������Ҳ�ᵼ��zΪ0�������ԡ�
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
//����߳̽�������a������ͨ��memory_order_relaxed�ķ�ʽ���ܱ�֤��a�Ĳ�����ԭ�ӵ�(ͬһʱ�̽���һ���߳�дa��ֵ�����ǿ��ܶ���̶߳�ȡa��ֵ)��


int main() {


	//TestSpinLock();
	TestOrderRelaxed();
	//TestOrderRelaxed2();

	return 0;


}