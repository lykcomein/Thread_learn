#pragma once
#include <mutex>
#include <memory>

//尾部插入链表
template<typename T>
class double_push_list 
{
	struct node_d
	{
		std::mutex m;
		std::shared_ptr<T> data;
		std::unique_ptr<node_d> next;
		node_d() :
			next()
		{}
		node_d(T const& value) :
			data(std::make_shared<T>(value))
		{}
	};

	node_d head;
	node_d* last_node_ptr;
	//控制最后一个节点指针更新
	std::mutex last_ptr_mtx; //保证线程串行

public:
	double_push_list() {
		last_node_ptr = &head;
	}
	~double_push_list()
	{
		remove_if([](node_d const&) {return true; });
	}

	double_push_list(double_push_list const& other) = delete;
	double_push_list& operator=(double_push_list const& other) = delete;

	//与头部插入不同之处：这里还对尾部node进行了操作，所以要lock尾部元素
	void push_back(T const& value) {
		//防止push_head同时进行
		//保证头部或删除节点更新last_node_ptr唯一, 所以同时加锁
		std::unique_ptr<node_d> new_node(new node_d(value));
		std::lock(last_node_ptr->m, last_ptr_mtx);
		std::unique_lock<std::mutex> lk(last_node_ptr->m, std::adopt_lock);
		std::unique_lock<std::mutex> last_lk(last_ptr_mtx, std::adopt_lock);
		//原来的最后节点的下一个节点指向新生成的节点
		last_node_ptr->next = std::move(new_node);
		//将最后一个节点后移
		last_node_ptr = last_node_ptr->next.get();
	}

	//将新节点插入到头结点后
	void push_front(T const& value)
	{
		std::unique_ptr<node_d> new_node(new node_d(value));
		std::lock_guard<std::mutex> lk(head.m);
		new_node->next = std::move(head.next);
		head.next = std::move(new_node);
		//如果新插入节点的next为nullptr，则更新last_node_ptr
		if (head.next->next == nullptr) {
			std::lock_guard<std::mutex> last_lk(last_ptr_mtx);
			last_node_ptr = head.next.get();
		}
	}

	template<typename Function>
	void for_each(Function f)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			lk.unlock();
			f(*next->data);
			current = next;
			lk = std::move(next_lk);
		}
	}

	template<typename Predicate>
	std::shared_ptr<T> find_first_if(Predicate p)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			lk.unlock();
			if (p(*next->data))
			{
				return next->data;
			}
			current = next;
			lk = std::move(next_lk);
		}
		return std::shared_ptr<T>();
	}

	template<typename Predicate>
	void remove_if(Predicate p)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			if (p(*next->data))
			{
				std::unique_ptr<node_d> old_next = std::move(current->next);
				current->next = std::move(next->next);
				//判断删除的是否为最后一个节点
				if (current->next == nullptr) {
					std::lock_guard<std::mutex> last_lk(last_ptr_mtx);
					last_node_ptr = current;
				}
				next_lk.unlock();
			}
			else
			{
				lk.unlock();
				current = next;
				lk = std::move(next_lk);
			}
		}
	}
	template<typename Predicate>
	bool remove_first(Predicate p)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			if (p(*next->data))
			{
				std::unique_ptr<node_d> old_next = std::move(current->next);
				current->next = std::move(next->next);
				//判断删除的是否为最后一个节点
				if (current->next == nullptr) {
					std::lock_guard<std::mutex> last_lk(last_ptr_mtx);
					last_node_ptr = &current;
				}
				next_lk.unlock();
				return true;
			}
			lk.unlock();
			current = next;
			lk = std::move(next_lk);
		}
		return false;
	}
	//从断言处查询满足条件的第一个点之前插入
	template<typename Predicate>
	void insert_if(Predicate p, T const& value)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			if (p(*(next->data)))
			{
				std::unique_ptr<node_d> new_node(new node_d(value));

				//缓存当前节点的下一个节点作为旧节点
				auto old_next = std::move(current->next);

				//新节点指向当前节点的下一个节点
				new_node->next = std::move(old_next);
				//当前节点的下一个节点更新为新节点
				current->next = std::move(new_node);
				return;
			}
			lk.unlock();
			current = next;
			lk = std::move(next_lk);
		}
	}

};