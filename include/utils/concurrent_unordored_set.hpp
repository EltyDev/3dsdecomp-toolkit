#ifndef CONCURRENT_UNORDORED_SET_HPP_
#define CONCURRENT_UNORDORED_SET_HPP_

#include <unordered_set>
#include <shared_mutex>

template<typename T>
class ConcurrentUnorderedSet
{
    public:

        void insert(const std::vector<T>& values) {
            std::unique_lock lock(_mutex);
            _set.insert(values.begin(), values.end());
        }

        bool insert(const T& value) {
            std::unique_lock lock(_mutex);
            return _set.insert(value).second;
        }

        bool contains(const T& value) {
            std::shared_lock lock(_mutex);
            return _set.contains(value);
        }

        bool empty() {
            std::shared_lock lock(_mutex);
            return _set.empty();
        }
        
        size_t size() {
            std::shared_lock lock(_mutex);
            return _set.size();
        }
    private:
        std::unordered_set<T> _set;
        mutable std::shared_mutex _mutex;
}; 

#endif /* !CONCURRENT_UNORDORED_SET_HPP_ */
