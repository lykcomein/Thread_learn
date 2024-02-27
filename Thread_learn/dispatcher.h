#pragma once
#include "message.h"

namespace messaging
{
	//��ʾ��رն��е���Ϣ
	class close_queue{};

	class dispatcher
	{
	private:
		queue* q;
		bool chained;
		//��dispatcher��ʵ�����ɸ���
		dispatcher(dispatcher const&) = delete;
		dispatcher& operator=(dispatcher const&) = delete;

		//��׼��TemplateDispatcher��ʵ�������ڲ�����
		template<
			typename Dispatcher,
			typename Msg,
			typename Func>
		friend class TemplateDispatcher;

		void wait_and_dispatch()
		{
			//������ѭ�����ȴ���Ϣ��������Ϣ
			for (;;) {
				auto msg = q->wait_and_pop();
				dispatch(msg);
			}
		}

		//��dispatch()�б���Ϣ�Ƿ�����close_queue���ͣ������ڣ����׳��쳣
		bool dispatch(std::shared_ptr<message_base> const& msg)
		{
			//shared_ptr.get()��ȡ���ڲ�ָ��(message_base*)
			if (dynamic_cast<wrapped_message<close_queue>*>(msg.get())) {
				throw close_queue();
			}
			return false;
		}

	public:
		//��dispatcher�ǿ��ƶ���ʵ��(�ƶ����캯��)
		dispatcher(dispatcher&& other) :q(other.q), chained(other.chained)
		{
			//�����ε���Ϣ�ַ��ⲻ��ȴ���Ϣ
			other.chained = true;
		}
		explicit dispatcher(queue* q_) : q(q_), chained(false) {}

		//�����TemplateDispatcher����ĳ�־������͵���Ϣ
		template<typename Message, typename Func, typename dispatcher>
		TemplateDispatcher<dispatcher, Message, Func>
			handle(Func&& f, std::string info_msg)
		{
			//std::cout << "Dispatcher  handle msg is " << info_msg << std::endl;
			return TemplateDispatcher<dispatcher, Message, Func>(
				q, this, std::forward<Func>(f), info_msg);
		}

		//���������������׳��쳣
		~dispatcher() noexcept(false) {
			if (!chained) {
				wait_and_dispatch();
			}
		}
	};

	class sender
	{
		//��sender���а�װ����Ϣ���е�ָ��
		queue* q;
	public:
		//����Ĭ�Ϸ�ʽ�����sender���ڲ���������
		sender() : q(nullptr) {}

		//�۸��ݶ���ָ�빹��senderʾ��
		explicit sender(queue* q_) : q(q_) {}
		template<typename Message>
		void send(Message const& msg)
		{
			if (q) {
				//�ܷ��Ͳ���������������Ϣ
				q->push(msg);
			}
		}
	};

	class receiver
	{
		//��receiverʵ����ȫӵ����Ϣ����
		queue q;
	public:
		//��receiver����׼����ʽת��Ϊsender����ǰ��ӵ�еĶ��б���������
		operator sender() {
			return sender(&q);
		}
		//�۶����ϵĵȴ���Ϊ�ᴴ��һ��dispatcher����
		dispatcher wait()
		{
			return dispatcher(&q);
		}
	};

	template<typename PreviousDispatcher, typename Msg, typename Func>
	class TemplateDispatcher
	{
		std::string _msg;
		queue* q;
		PreviousDispatcher* prev;
		Func f;
		bool chained;
		TemplateDispatcher(TemplateDispatcher const&) = delete;
		TemplateDispatcher& operator= (TemplateDispatcher const&) = delete;
		template<typename Dispatcher, typename OtherMsg, typename OtherFunc>
		//�ٸ�����ģ��TemplaDispatcher<>���ֻ����ɵĸ������ͻ�Ϊ����
		friend class TemplateDispatcher;

		void wait_and_dispatch()
		{
			for (;;) {
				auto msg = q->wait_and_pop();
				//�������Ϣ�����ƴ�������������ѭ��
				if (dispatch(msg)) {
					break;
				}
			}
		}
		bool dispatch(std::shared_ptr<message_base> const& msg)
		{
			//�۲�����Ϣ��𲢵�����Ӧ�Ĵ�����
			if (wrapped_message<Msg>* wrapper = dynamic_cast<wrapped_message<Msg>*>(msg.get())) {
				f(wrapper->contents);
				return true;
			}
			else {
				//���ν�ǰһ��dispatcher�����γ���������
				return prev->dispatch(msg);
			}
		}

	public:

		TemplateDispatcher(TemplateDispatcher&& other) :
			q(other.q), prev(other.prev), f(std::move(other.f)),
			chained(other.chained), _msg(other._msg)
		{
			//std::cout << "TemplateDispatcher copy construct msg is " << _msg << std::endl;
			other.chained = true;
		}

		TemplateDispatcher(queue* q_, PreviousDispatcher* prev_, Func&& f_, std::string msg) :
		q(q_), prev(prev_), f(std::forward<Func>(f_)), chained(false), _msg(msg){
			prev_->chained = true;
		}

		//�ݰ��νӳ����ķ�ʽ������ദ����
		template<typename OtherMsg, typename OtherFunc>
		TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc> 
			handle(OtherFunc&& of, std::string info_msg) {
			return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>(q, this, std::forward<OtherFunc>(of), info_msg);
		}

		//�޸���������������쳣��Ϊ����Ҳ��noexecpt(false)
		~TemplateDispatcher() noexcept(false) {
			if (!chained) {
				wait_and_dispatch();
			}
		}
	};
}