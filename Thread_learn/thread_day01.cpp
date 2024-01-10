#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <thread>
#include <sstream>
#include <vector>
#include <numeric>

void thread_work(std::string str)
{
    std::cout << "str is " << str << std::endl;
}

class background_task
{
public:
    void operator()() {
        std::cout << "background_task called" << std::endl;
    }
};

struct func
{
    int& _i;
    func(int& i) : _i(i) {}
    void operator()()
    {
        for (int i = 0; i < 3; i++)
        {
            _i = i;
            std::cout << "_i is" << _i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

void oops()
{
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread(myfunc);
    //���������ʾֲ��������ֲ��������ܻ�����}���������ջ��������߳��˳�������
    functhread.detach();
}

void use_join() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread(myfunc);
    functhread.join();
}
//����detach�����߳̾ͼ������У�ֱ��������������join�����̵߳ȴ����߳����н�����ż������У�

void catch_exception()
{
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread{ myfunc };
    try {
        //���߳���һЩ����
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (std::exception& e) {
        functhread.join();
        throw;
    }

    functhread.join();
}

class thread_guard
{
private:
    std::thread& _t;
public:
    explicit thread_guard(std::thread& t) :_t(t) {}
    ~thread_guard() {
        //joinֻ�ܵ���һ��
        if (_t.joinable())
        {
            _t.join();
        }
    }

    //ɾ��()�Լ�=������
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

void auto_guard()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);

    std::cout << "auto guard finished " << std::endl;
}

void print_str(int i, std::string const& s)
{
    std::cout << "i is " << i << " str is " << s << std::endl;
}

void danger_oops(int som_param)
{
    char buffer[1024];
    sprintf(buffer, "%i", som_param);
    //���߳��ڲ���char const* ת��Ϊstd::string
    //ָ�볣��  char * const p  ָ�뱾���ܱ�
    //����ָ��  const char * p ָ������ݲ��ܱ�    
    std::thread t(print_str, 3, buffer);
    t.detach();
    std::cout << "danger oops finished " << std::endl;
}

void safe_oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%i", some_param);
    std::thread t(print_str, 3, std::string(buffer));
    t.detach();
}

void change_param(int& param)
{
    param++;
}

//���߳�Ҫ���õĻص���������Ϊ��������ʱ����Ҫ��������ʾת��Ϊ���ö��󴫵ݸ��̵߳Ĺ��캯����
void ref_oops(int some_param)
{
    std::cout << "before change, param is " << some_param << std::endl;
    //��Ҫʹ��������ʾת��
    // std::thread t2(change_param, some_param); //�ᱨ��
    std::thread  t2(change_param, std::ref(some_param));
    t2.join();
    std::cout << "after change , param is " << some_param << std::endl;
}

class X
{
public:
    void do_lengthy_work()
    {
        std::cout << "do_lengthy_work " << std::endl;
    }
};

void bind_class_oops()
{
    X my_x;
    std::thread t(&X::do_lengthy_work, &my_x);
    t.join();
}

void deal_unique(std::unique_ptr<int> p)
{
    std::cout << "unique ptr data is " << *p << std::endl;
    (*p)++;

    std::cout << "after unique ptr data is " << *p << std::endl;
}

void move_oops() {
    auto p = std::make_unique<int>(100);
    std::thread  t(deal_unique, std::move(p));
    t.join();
    //������ʹ��p�ˣ�p�Ѿ���move����
    // std::cout << "after unique ptr data is " << *p << std::endl;
}

int day01() {
    std::string hellostr = "hello world!";
    //1 ͨ��()��ʼ��������һ���߳�
    std::cout << hellostr << std::endl;
    std::thread t1(thread_work, hellostr);
    //2 ���̵߳ȴ����߳��˳�
    t1.join();
    //3 t2��������������Ķ��壬������Ϊ����std::thread,����Ϊbackground_task;
    //std::thread t2(background_task());
    //t2.join();
    //�ɶ��һ��()
    std::thread t2((background_task()));
    t2.join();

    //ʹ��{}������ʼ��
    std::thread t3{ background_task() };
    t3.join();

    //lambda���ʽ
    std::thread t4([](std::string str) {
        std::cout << "str is " << str << std::endl;
        }, hellostr);
    t4.join();

    //detachע������
    oops();
    //��ֹ���߳��˳����죬ͣ��һ�£������߳�������detach
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //6 join�÷�
    use_join();

    //7 �����쳣
    catch_exception();

    //8 �Զ�����
    auto_guard();

    //Σ�տ��ܴ��ڱ���
    danger_oops(100);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    //��ȫ����ǰת��
    safe_oops(100);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    //������
    ref_oops(100);

    //����ĳ�Ա����
    bind_class_oops();

    //ͨ��move���ݲ���

    move_oops();
    return 0;
}

void some_function() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
void some_other_function() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void dangerous_use() {
    //t1 ��some_function
    std::thread t1(some_function);
    //2 ת��t1������̸߳�t2��ת�Ժ�t1��Ч
    std::thread t2 = std::move(t1);
    //3 t1���Լ����������̣߳�ִ��some_other_function
    t1 = std::thread(some_other_function);
    //4 ����һ���̱߳���t3
    std::thread t3;
    //5 ת��t2������̸߳�t3
    t3 = std::move(t2);
    //6 ת��t3������̸߳�t1(��ʱt1���ڹ����߳�����some_other_function���ᴥ��terminate������������)
    t1 = std::move(t3);
    std::this_thread::sleep_for(std::chrono::seconds(2000));
}

std::thread f() {
    return std::thread(some_function);
}

void param_function(int a) {
    while (true) {
        std::cout << "param is " << a << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::thread g() {
    std::thread t(param_function, 43);
    return t;
}

class joining_thread {
    std::thread _t;
public:
    joining_thread() noexcept = default;
    template<typename Callable, typename ... Args>
    explicit joining_thread(Callable&& func, Args && ...args) :_t(std::forward<Callable>(func), std::forward<Args>(args)...) {}
    explicit joining_thread(std::thread t)noexcept : _t(std::move(t)) {}
    joining_thread(joining_thread&& other) noexcept : _t(std::move(other._t)) {}

    joining_thread& operator=(joining_thread&& other) noexcept {
        //�����ǰ�߳̿ɻ�ϣ����ϵȴ��߳�����ٸ�ֵ
        if (_t.joinable()) {
            _t.join();
        }
        _t = std::move(other._t);
        return *this;
    }
    // joining_thread& operator=(joining_thread other) noexcept
    // {
    //     //�����ǰ�߳̿ɻ�ϣ����ϵȴ��߳�����ٸ�ֵ
    //     if (_t.joinable()) {
    //         _t.join();
    //     }
    //     _t = std::move(other._t);
    //     return *this;
    // }

    ~joining_thread() noexcept {
        if (_t.joinable()) {
            _t.join();
        }
    }

    void swap(joining_thread& other) noexcept {
        _t.swap(other._t);
    }

    std::thread::id   get_id() const noexcept {
        return _t.get_id();
    }

    bool joinable() const noexcept {
        return _t.joinable();
    }

    void join() {
        _t.join();
    }

    void detach() {
        _t.detach();
    }

    std::thread& as_thread() noexcept {
        return _t;
    }

    const std::thread& as_thread() const noexcept {
        return _t;
    }

};

void use_jointhread() {
    //1 �����̹߳��캯������joiningthread
    joining_thread j1([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id() << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10);
    //2 ����thread����joiningthread
    joining_thread j2(std::thread([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id() << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10));
    //3 ����thread����j3
    joining_thread j3(std::thread([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id()
                << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10));
    //4 ��j3��ֵ��j1,joining_thread�ڲ���ȴ�j1��Ͻ������ٽ�j3��ֵ��j1
    j1 = std::move(j3);

}

void use_vector() {
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < 10; i++) {
        threads.emplace_back(param_function, i);
    }

    for (auto& entry : threads) {
        entry.join();
    }
}

template<typename Iterator, typename T>
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T& result) {
        result = std::accumulate(first, last, result);
    }
};

//ѡ����������
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);
    //distance:��������������֮��ľ���
    if (!length) {
        return init;//�ж�Ҫ�����������Ԫ��Ϊ0���򷵻�
    }
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread; //��������߳�����Ԥ��ÿ���̼߳���25�����ݳ���
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads); //�����ʺϿ��ٵ��߳�������Сֵ
    unsigned long const block_size = length / num_threads; //�����˲��������ݲ����ƶ�������Ȼ�󿪱��̼߳���
    std::vector<T> results(num_threads);
    std::vector<std::thread>  threads(num_threads - 1); //��ʼ�����߳���-1����С��vector����Ϊ���߳�Ҳ������㣬��������-1.
    Iterator block_start = first;

    for (unsigned long i = 0; i < (num_threads - 1); i++) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size); //�ƶ�����
        threads[i] = std::thread(accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i])); //�����߳�
        block_start = block_end; //������ʼλ��
    }
    accumulate_block<Iterator, T>()(
        block_start, last, results[num_threads - 1]); //���̼߳���
    for (auto& entry : threads)
        entry.join();    //�������߳�join
    return std::accumulate(results.begin(), results.end(), init); //�����м������ٴε���std��accumulate��������

}

void use_parallel_acc() {
    std::vector <int> vec(10000);
    for (int i = 0; i < 10000; i++) {
        vec.push_back(i);
    }
    int sum = 0;
    sum = parallel_accumulate<std::vector<int>::iterator, int>(vec.begin(),
        vec.end(), sum);

    std::cout << "sum is " << sum << std::endl;
}
void day02() {
    //dangerous_use();
   // use_jointhread();
    //use_vector();
    use_parallel_acc();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}


int main() {
    // day01();
    day02();
    return 0;
}

