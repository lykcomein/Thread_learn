#pragma once
#include <mutex>
#include <memory>

//线程安全链表，头部插入
template<typename T>
class threadsafe_list 
{
	struct node
	{
		std::mutex m;
		std::shared_ptr<T> data;
		std::unique_ptr<node> next;
		node() : next(){}
		node(T const& value) : data(std::make_shared<T>(value)){}
	};

	node head;
public:
	threadsafe_list(){}
	~threadsafe_list(){
		remove_if([](node const&) {
			return true;
			});
	}

	threadsafe_list(threadsafe_list const& other) = delete;
	threadsafe_list& operator=(threadsafe_list const& other) = delete;

	template<typename Predicate>
	void remove_if(Predicate p) {
		node* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node* const next = current->next.get()) {
			std::unique_lock<std::mutex> next_lk(next->m);
			if (p(*next->data)) //满足p规则
			{
				std::unique_ptr<node> old_next = std::move(current->next);
				current->next = std::move(next->next);
				next_lk.unlock();
			}
			else {
				lk.unlock();
				current = next;
				lk = std::move(next_lk);
			}
		}
	}

	void push_front(T const& value) {
		std::unique_ptr<node> new_node(new node(value));
		std::lock_guard<std::mutex> lk(head.m);
		new_node->next = std::move(head.next);
		head.next = std::move(new_node);
	}

	template<typename Predicate>
	std::shared_ptr<T> find_first_if(Predicate p) {
		node* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node* const next = current->next.get()) {
			std::unique_lock<std::mutex> next_lk(next->m);
			lk.unlock();
			if (p(*next->data)) {
				return next->data;
			}
			current = next;
			lk = std::move(next_lk);
		}
		return std::shared_ptr<T>();
	}
	//遍历
	template<typename Function>
	void for_each(Function f)
	{
		node* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			lk.unlock();
			f(*next->data);
			current = next;
			lk = std::move(next_lk);
		}
	}
};