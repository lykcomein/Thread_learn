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
    //隐患，访问局部变量，局部变量可能会随着}结束而回收或随着主线程退出而回收
    functhread.detach();
}

void use_join() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread(myfunc);
    functhread.join();
}
//调用detach后，主线程就继续运行，直到结束；而调用join后，主线程等待子线程运行结束后才继续运行；

void catch_exception()
{
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread{ myfunc };
    try {
        //本线程做一些事情
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
        //join只能调用一次
        if (_t.joinable())
        {
            _t.join();
        }
    }

    //删除()以及=函数；
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
    //在线程内部将char const* 转化为std::string
    //指针常量  char * const p  指针本身不能变
    //常量指针  const char * p 指向的内容不能变    
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

//当线程要调用的回调函数参数为引用类型时，需要将参数显示转化为引用对象传递给线程的构造函数，
void ref_oops(int some_param)
{
    std::cout << "before change, param is " << some_param << std::endl;
    //需要使用引用显示转换
    // std::thread t2(change_param, some_param); //会报错
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
    //不能再使用p了，p已经被move废弃
    // std::cout << "after unique ptr data is " << *p << std::endl;
}

int day01() {
    std::string hellostr = "hello world!";
    //1 通过()初始化并启动一个线程
    std::cout << hellostr << std::endl;
    std::thread t1(thread_work, hellostr);
    //2 主线程等待子线程退出
    t1.join();
    //3 t2被当作函数对象的定义，其类型为返回std::thread,参数为background_task;
    //std::thread t2(background_task());
    //t2.join();
    //可多加一层()
    std::thread t2((background_task()));
    t2.join();

    //使用{}方法初始化
    std::thread t3{ background_task() };
    t3.join();

    //lambda表达式
    std::thread t4([](std::string str) {
        std::cout << "str is " << str << std::endl;
        }, hellostr);
    t4.join();

    //detach注意事项
    oops();
    //防止主线程退出过快，停顿一下，让子线程跑起来detach
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //6 join用法
    use_join();

    //7 捕获异常
    catch_exception();

    //8 自动守卫
    auto_guard();

    //危险可能存在崩溃
    danger_oops(100);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    //安全，提前转化
    safe_oops(100);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    //绑定引用
    ref_oops(100);

    //绑定类的成员函数
    bind_class_oops();

    //通过move传递参数

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
    //t1 绑定some_function
    std::thread t1(some_function);
    //2 转移t1管理的线程给t2，转以后t1无效
    std::thread t2 = std::move(t1);
    //3 t1可以继续绑定其他线程，执行some_other_function
    t1 = std::thread(some_other_function);
    //4 创建一个线程变量t3
    std::thread t3;
    //5 转移t2管理的线程给t3
    t3 = std::move(t2);
    //6 转移t3管理的线程给t1(此时t1正在管理线程运行some_other_function，会触发terminate函数引发崩溃)
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
        //如果当前线程可汇合，则汇合等待线程完成再赋值
        if (_t.joinable()) {
            _t.join();
        }
        _t = std::move(other._t);
        return *this;
    }
    // joining_thread& operator=(joining_thread other) noexcept
    // {
    //     //如果当前线程可汇合，则汇合等待线程完成再赋值
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
    //1 根据线程构造函数构造joiningthread
    joining_thread j1([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id() << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10);
    //2 根据thread构造joiningthread
    joining_thread j2(std::thread([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id() << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10));
    //3 根据thread构造j3
    joining_thread j3(std::thread([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id()
                << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10));
    //4 把j3赋值给j1,joining_thread内部会等待j1汇合结束后再将j3赋值给j1
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

//选择运行数量
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);
    //distance:返回两个迭代器之间的距离
    if (!length) {
        return init;//判断要计算的容器内元素为0个则返回
    }
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread; //开辟最大线程数，预估每个线程计算25个数据长度
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads); //计算适合开辟的线程数的最小值
    unsigned long const block_size = length / num_threads; //计算了步长，根据步长移动迭代器然后开辟线程计算
    std::vector<T> results(num_threads);
    std::vector<std::thread>  threads(num_threads - 1); //初始化了线程数-1个大小的vector，因为主线程也参与计算，所以这里-1.
    Iterator block_start = first;

    for (unsigned long i = 0; i < (num_threads - 1); i++) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size); //移动步长
        threads[i] = std::thread(accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i])); //开辟线程
        block_start = block_end; //更新起始位置
    }
    accumulate_block<Iterator, T>()(
        block_start, last, results[num_threads - 1]); //主线程计算
    for (auto& entry : threads)
        entry.join();    //让所有线程join
    return std::accumulate(results.begin(), results.end(), init); //将所有计算结果再次调用std的accumulate算出结果。

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

