#pragma once

#include <iostream>
#include <queue>
#include <any>
#include <mutex>
#include <condition_variable>


namespace ak
{
	template <typename T>
	class SafeThreadQueue final
	{
	public:
		bool empty() const;
		void push(T elem);
		T pop();

	private:
		struct
		{
			std::queue<T> queue_;
			mutable std::mutex mtx_{};
		} state_;
		std::condition_variable condVar_{};
	};

	template <typename T>
	inline bool SafeThreadQueue<T>::empty() const
	{
		std::unique_lock<std::mutex> ulMtx(state_.mtx_);
		return state_.queue_.empty();
	}

	template <typename T>
	inline void SafeThreadQueue<T>::push(T elem)
	{
		std::unique_lock<std::mutex> ulMtx(state_.mtx_);
		state_.queue_.push(std::move(elem));
		condVar_.notify_one();
	}

	template <typename T>
	inline T SafeThreadQueue<T>::pop()
	{
		std::unique_lock<std::mutex> ulMtx(state_.mtx_);
		condVar_.wait(ulMtx, [&]() { return !state_.queue_.empty(); });

		auto ret = std::move(state_.queue_.front());
		state_.queue_.pop();
		return ret;
	}
}