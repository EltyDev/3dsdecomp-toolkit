#ifndef CONCURRENT_VECTOR_HPP_
#define CONCURRENT_VECTOR_HPP_

#include <vector>
#include <shared_mutex>

template<typename T>
class ConcurrentVector
{
    public:

        ConcurrentVector(size_t initialSize = 0) : _vector(initialSize) {}
        
        void push_back(const T& value) {
            std::unique_lock lock(_mutex);
            _vector.push_back(value);
        }

        template<typename... Args>
        void emplace_back(Args&&... args) {
            std::unique_lock lock(_mutex);
            _vector.emplace_back(std::forward<Args>(args)...);
        }

        bool contains(const T& value) {
            std::shared_lock lock(_mutex);
            return std::find(_vector.begin(), _vector.end(), value) != _vector.end();
        }

        bool empty() {
            std::shared_lock lock(_mutex);
            return _vector.empty();
        }
        
        size_t size() {
            std::shared_lock lock(_mutex);
            return _vector.size();
        }

        std::vector<T> & operator*() {
            return _vector;
        }

        T & operator[](size_t index) {
            std::shared_lock lock(_mutex);
            return _vector[index];
        }

        void insert(typename std::vector<T>::iterator pos, std::vector<T>::iterator first, std::vector<T>::iterator last) {
            std::unique_lock lock(_mutex);
            _vector.insert(pos, first, last);
        }

        void insert(std::vector<T>::iterator first, std::vector<T>::iterator last) {
            std::unique_lock lock(_mutex);
            _vector.insert(_vector.end(), first, last);
        }

        std::vector<T>::iterator begin() {
            return _vector.begin();
        }

        std::vector<T>::iterator end() {
            return _vector.end();
        }

        void clear() {
            std::unique_lock lock(_mutex);
            _vector.clear();
        }

    private:
        std::vector<T> _vector;
        mutable std::shared_mutex _mutex;
};

#endif /* !CONCURRENT_VECTOR_HPP_ */
