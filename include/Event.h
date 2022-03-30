#pragma once

#include <vector>
#include <functional>
#include <ranges>

template<typename Func, typename... Args> requires std::invocable<Func, Args...>
class EventBase
{
public:
	using CallbackType = Func;

	EventBase(const EventBase&) = delete;
	EventBase(EventBase&&) = delete;
	EventBase& operator=(const EventBase&) = delete;
	EventBase& operator=(EventBase&&) = delete;

	int AddCallback(CallbackType&& function, int priority = 0)
	{
		callbacks_.push_back({ callbackNextID_++, priority, std::move(function) });
		std::ranges::sort(callbacks_, [](auto& a, auto& b) { return a.priority > b.priority; });
		return callbacks_.back().id;
	}

	void RemoveCallback(int id)
	{
		auto it = std::ranges::find_if(callbacks_, [id](auto& cb) { return cb.id == id; });
		if (it != callbacks_.end())
			callbacks_.erase(it);
	}

protected:
	struct Callback
	{
		int id;
		int priority;
		std::function<CallbackType> callback;
	};

	int callbackNextID_ = 0;
	std::vector<Callback> callbacks_;
};

template<typename Func, typename... Args>
concept NotVoidFunction = std::invocable<Func, Args...> && !std::is_void_v<std::invoke_result_t<Func, Args...>>;

template<typename Func, typename... Args>
class Event
{

};

template<typename Func, typename... Args> requires NotVoidFunction<Func, Args...>
class Event<Func, Args...> : public EventBase<Func, Args...>
{
public:
	using DowncastType = EventBase<Func, Args...>;
	using CallbackType = Func;
	using ReturnType = std::invoke_result_t<Func, Args...>;
	using CombineFunc = std::function<ReturnType(ReturnType&, ReturnType&)>;

	Event(CombineFunc&& combine = std::logical_or<ReturnType>{})
		: combine_(std::move(combine))
	{ }

	DowncastType& Downcast() { return *this; }

	ReturnType operator()(Args ...args)
	{
		if (this->callbacks_.empty())
			return { };

		ReturnType rval = this->callbacks_[0].callback(std::forward<Args>(args)...);
		for (size_t i = 1; i < this->callbacks_.size(); i++)
			rval = combine_(rval, this->callbacks_[i].callback(std::forward<Args>(args)...));

		return rval;
	}

protected:
	CombineFunc combine_;
};

template<typename Func, typename... Args>
concept VoidFunction = std::is_void_v<std::invoke_result_t<Func, Args...>>;

template<typename Func, typename... Args> requires VoidFunction<Func, Args...>
class Event<Func, Args...> : public EventBase<Func, Args...>
{
public:
	using DowncastType = EventBase<Func, Args...>;
	using CallbackType = Func;

	Event() = default;

	DowncastType& Downcast() { return *this; }

	void operator()(Args ...args)
	{
		if (this->callbacks_.empty())
			return;

		for (auto& cb : this->callbacks_)
			cb.callback(std::forward<Args>(args)...);
	}
};

