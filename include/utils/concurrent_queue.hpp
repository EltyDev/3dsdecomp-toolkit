#ifndef CONCURRENT_QUEUE_HPP_
#define CONCURRENT_QUEUE_HPP_

#include <mutex>
#include <queue>
#include <shared_mutex>

template<typename T>
class ConcurrentQueue
{
    public:

        ConcurrentQueue() = default;

        void push(const T& value) {
            std::unique_lock lock(_mutex);
            _queue.push(value);
        }

        bool try_pop(T& value) {
            std::unique_lock lock(_mutex);
            if (_queue.empty())
                return false;
            value = _queue.front();
            _queue.pop();
            return true;
        }

        bool empty() {
            std::shared_lock lock(_mutex);
            return _queue.empty();
        }
        
        size_t size() {
            std::shared_lock lock(_mutex);
            return _queue.size();
        }

    private:
        std::queue<T> _queue;
        mutable std::shared_mutex _mutex;
};

#endif /* !CONCURRENT_QUEUE_HPP_ */
