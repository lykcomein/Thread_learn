//����������future��promise��async
#include <iostream>
#include <future>
#include <chrono>

//std::async ��һ�������첽ִ�к�����ģ�庯����������һ�� std::future ���󣬸ö������ڻ�ȡ�����ķ���ֵ��

//����һ���첽����
std::string fetchDataFromDB(std::string query) {
	//ģ��һ���첽���񣬱�������ݿ��л�ȡ����
	std::this_thread::sleep_for(std::chrono::seconds(5));
	return "Data: " + query;
}

void use_async() {
	//ʹ��std::async�첽����fetchDataFromDB
	std::future<std::string> resultFromDB = std::async(std::launch::async, fetchDataFromDB, "Data");

	//�����߳�������������
	std::cout << "Doing something else..." << std::endl;

	//��future�����ȡ����
	std::string dbData = resultFromDB.get();
	std::cout << dbData << std::endl;

}
/*
* �����ʾ���У�std::async ������һ���µ��̣߳�����ڲ��̳߳�����ѡһ���̣߳����Զ���һ�� std::promise �����������
* std::promise ���󱻴��ݸ� fetchDataFromDB �����������ķ���ֵ���洢�� std::future �����С�
* ��ʹ�� std::async ������£����Ǳ���ʹ�� std::launch::async ��־����ȷ��������ϣ�������첽ִ�С�
* �����߳��У����ǿ���ʹ�� std::future::get ������ std::future �����л�ȡ���ݡ�
*/

/*
* async����������
* std::async�������Խ��ܼ�����ͬ���������ԣ���Щ������std::launchö���ж��塣
* ����std::launch::async֮�⣬���������������ԣ�
* std::launch::deferred�����ֲ�����ζ�������ڵ���std::future::get()��std::future::wait()����ʱ�ӳ�ִ�С�
* ���仰˵����������Ҫ���ʱͬ��ִ�С�
* std::launch::async | std::launch::deferred�����ֲ����������������Ե���ϡ�
* ���������һ���������߳����첽ִ�У�Ҳ�����ӳ�ִ�У�����ȡ����ʵ�֡�
* 
* Ĭ������£�std::asyncʹ��std::launch::async | std::launch::deferred���ԡ�
* ����ζ����������첽ִ�У�Ҳ�����ӳ�ִ�У�����ȡ����ʵ�֡�
* ��Ҫע����ǣ���ͬ�ı������Ͳ���ϵͳ���ܻ��в�ͬ��Ĭ����Ϊ��
** 
*/


/*
* std::future::get() �� std::future::wait()
* 1��std::futurn::get()
* std::future::get() ��һ���������ã����ڻ�ȡ std::future �����ʾ��ֵ���쳣��
* ����첽����û����ɣ�get()��������ǰ�̣߳�ֱ��������ɡ���������Ѿ���ɣ�get()��������������Ľ����
* ��Ҫ���ǣ�get() ֻ�ܵ���һ�Σ���Ϊ�����ƶ������ĵ� std::future �����״̬��
* һ�� get() �����ã�std::future ����Ͳ����ٱ�������ȡ�����
* 2��std::futurn::wait()
* std::future::wait() Ҳ��һ���������ã������� get() ����Ҫ�������� wait() ���᷵������Ľ����
* ��ֻ�ǵȴ��첽������ɡ���������Ѿ���ɣ�wait() ���������ء�
* �������û����ɣ�wait() ��������ǰ�̣߳�ֱ��������ɡ�
* �� get() ��ͬ��wait() ���Ա���ε��ã����������ĵ� std::future �����״̬
*/

/*
* �ܽ�һ�£���������������Ҫ�������ڣ�
* std::future::get() ���ڻ�ȡ����������Ľ������ std::future::wait() ֻ�ǵȴ�������ɡ�
* get() ֻ�ܵ���һ�Σ��� wait() ���Ա���ε��á�
* �������û����ɣ�get() �� wait() ����������ǰ�̣߳��� get() ��һֱ����ֱ��������ɲ����ؽ������ wait() ֻ���ڵȴ�������ɡ�
*/

/*
* std::packaged_task��std::future��C++11������������࣬�������ڴ����첽����Ľ����
* std::packaged_task��һ���ɵ���Ŀ�꣬����װ��һ�����񣬸������������һ���߳������С������Բ�������ķ���ֵ���쳣��������洢��std::future�����У��Ա��Ժ�ʹ�á�
* std::future����һ���첽�����Ľ�������������ڴ��첽�����л�ȡ����ֵ���쳣��
*/

int my_task() {
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::cout << "my task run 5 s" << std::endl;
	return 42;
}

void use_package() {
	//����һ����װ�������std::package_tack����
	std::packaged_task<int()> task(my_task);

	//��ȡ�����������std::future����
	std::future<int> result = task.get_future();

	//����һ���߳���ִ������
	std::thread t(std::move(task));
	t.detach(); //���߳������̷߳��룬�Ա����߳̿��Եȴ��������

	//�ȴ�������ɲ���ȡ���
	int value = result.get();
	std::cout << "The result is: " << value << std::endl;

}

/*
* std::promise������ĳһ�߳�������ĳ��ֵ���쳣
*/

void set_value(std::promise<int> prom) {
	//����promise��ֵ
	std::this_thread::sleep_for(std::chrono::seconds(5));
	prom.set_value(10);
	std::cout << "promise set value success" << std::endl;
}

void use_promise() {
	//����һ��promise����
	std::promise<int> prom;
	//��ȡ��promise�������future����
	std::future<int> fut = prom.get_future();
	//�����߳�������promise��ֵ
	std::thread t(set_value, std::move(prom));
	//t.join();
	//�����߳��л�ȡfuture��ֵ
	std::cout << "Waiting for the thread to set the value..." << std::endl;
	std::cout << "Value set by the thread: " << fut.get() << std::endl;
	t.join();

}

void set_exception(std::promise<void> prom) {
	try {
		// �׳�һ���쳣
		throw std::runtime_error("An error occurred!");
	}
	catch (...) {
		// ���� promise ���쳣
		prom.set_exception(std::current_exception());
	}
}

void use_promise_exception() {
	std::promise<void> prom;
	// ��ȡ�� promise ������� future ����
	std::future<void> fut = prom.get_future();
	// �����߳������� promise ���쳣
	std::thread t(set_exception, std::move(prom));
	// �����߳��л�ȡ future ���쳣
	try {
		std::cout << "Waiting for the thread to set the exception...\n";
		fut.get();
	}
	catch (const std::exception& e) {
		std::cout << "Exception set by the thread: " << e.what() << '\n';
	}
	t.join();
}

void use_promise_destruct() {
	std::thread t;
	std::future<int> fut;
	{
		//����һ��promise����
		std::promise<int> prom;
		//��ȡ��promise��ص�future����
		fut = prom.get_future();
		//�����߳�������promise��ֵ
		t = std::thread(set_value, std::move(prom));
	}

	//�����߳��л�ȡfuture��ֵ
	std::cout << "Waiting for the thread to set the value..." << std::endl;
	std::cout << "Value set by the thread: " << fut.get() << std::endl;
	t.join();
}

void myFunction(std::promise<int>&& promise) {
	//ģ��һЩ����
	std::this_thread::sleep_for(std::chrono::seconds(1));
	promise.set_value(42);
}

//�����͵�future
void threadFunction(std::shared_future<int> future) {
	try {
		int result = future.get();
		std::cout << "Result: " << result << std::endl;
	}
	catch (const std::future_error& e) {
		std::cout << "Future error: " << e.what() << std::endl;
	}
}

void use_shared_future() {
	std::promise<int> promise;
	std::shared_future<int> future = promise.get_future();
	std::thread myThread1(myFunction, std::move(promise));//��promise�ƶ����߳���

	//ʹ��share()������ȡ�µ�shared_future����
	std::thread myThread2(threadFunction, future);

	std::thread myThread3(threadFunction, future);

	std::cout <<"The promis reback: " << future.get() << std::endl;

	myThread1.join();
	myThread2.join();
	myThread3.join();
}
//һ��future���ƶ�������shared_future�Ǵ����
void use_shared_future_error() {
	std::promise<int> promise;
	std::shared_future<int> future = promise.get_future();

	std::thread myThread1(myFunction, std::move(promise)); // �� promise �ƶ����߳���

	// ʹ�� share() ������ȡ�µ� shared_future ����  

	std::thread myThread2(threadFunction, std::move(future));

	std::thread myThread3(threadFunction, std::move(future));

	myThread1.join();
	myThread2.join();
	myThread3.join();
}

void may_throw() {
	//���������׳�һ���쳣����ʵ�ʵĳ����У���������κεط�����
	throw std::runtime_error("Oops, something went wrong!");
}

void use_funture_exception() {
	//����һ���첽����
	std::future<void> result(std::async(std::launch::async, may_throw));

	try {
		//��ȡ���(����ڻ�ȡ���ʱ������һ������ô�������׳��쳣)
		result.get();
	}
	catch (const std::exception& e) {
		//���񲢴�ӡ�쳣
		std::cerr << "Caught exception: " << e.what() << std::endl;
	}
}

int main() {
	//use_async();
	//my_task();
	//use_package();
	//use_promise();
	//use_promise_exception();
	//use_shared_future();
	use_funture_exception();
	return 0;
}