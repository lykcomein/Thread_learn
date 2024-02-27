#pragma once
#include <mutex>
#include <functional>
#include <iostream>
#include <condition_variable>
#include <queue>
#include <memory>
namespace messaging
{
	//����Ϣ���ࡣ�����д洢����Ŀ
	struct message_base
	{
		virtual ~message_base() {};
	};

	//��ÿ����Ϣ�����ػ�����
	template<typename Msg>
	struct wrapped_message:message_base
	{
		Msg contents;
		explicit wrapped_message(Msg const& contents_) : contents(contents_){}
	};

	//����Ϣ����(�����½���һ��queue�࣬��std::queue��ͬ)
	class queue 
	{
		std::mutex m;
		std::condition_variable c;
		//���ڲ����д洢message_base�͹���ָ��
		std::queue<std::shared_ptr<message_base>> q;
	public:
		template<typename T>
		void push(T const& msg) 
		{
			std::lock_guard<std::mutex> lk(m);
			//�ݰ�װ��������Ϣ�����洢���ָ��
			q.push(std::make_shared<wrapped_message<T>>(msg));
			c.notify_all();
		}
		std::shared_ptr<message_base> wait_and_pop()
		{
			std::unique_lock<std::mutex> lk(m);
			//���������Ϊ�գ��ͷ�������
			c.wait(lk, [&]() {
				return !q.empty(); 
				});
			auto res = q.front();
			q.pop();
			return res;
		}
	};
}
