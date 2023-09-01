#ifndef SRC_COMMON_THREAD_BLOCKINGQUEUE_H_
#define SRC_COMMON_THREAD_BLOCKINGQUEUE_H_
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
template <typename T>
class BlockingQueue {
   public:
    BlockingQueue();
    void push(const T &t);
    void push(const T &&t);
    T pop(const char *log_waiting_msg = "");
    T peek();
    void clear();
    size_t size();
    bool try_pop(T *t);
    bool try_peek(T *t);
    class Sync {
       public:
        mutable std::mutex mutex_;
        std::condition_variable condvar_;
    };

   private:
    std::queue<T> Q_;
    std::shared_ptr<Sync> sync_;
};

template <typename T>
BlockingQueue<T>::BlockingQueue() : sync_(new Sync()) {}

template <typename T>
void BlockingQueue<T>::clear() {
    std::unique_lock<std::mutex> lock(sync_->mutex_);
    std::queue<T> empty;
    Q_.swap(empty);
}

template <typename T>
void BlockingQueue<T>::push(const T &t) {
    std::unique_lock<std::mutex> lock(sync_->mutex_);
    Q_.push(t);
    sync_->condvar_.notify_one();
}

template <typename T>
void BlockingQueue<T>::push(const T &&t) {
    std::unique_lock<std::mutex> lock(sync_->mutex_);
    Q_.push(t);
    sync_->condvar_.notify_one();
}

template <typename T>
T BlockingQueue<T>::pop(const char *log_waiting_msg) {
    std::unique_lock<std::mutex> lock(sync_->mutex_);
    while (Q_.empty()) {
        // if (log_waiting_msg) {
        //     // fprintf(stderr, "%s\n", log_waiting_msg);
        // }
        // suspend, spare CPU clock
        sync_->condvar_.wait(lock);
    }
    T t = Q_.front();
    Q_.pop();
    return t;
}

template <typename T>
T BlockingQueue<T>::peek() {
    std::unique_lock<std::mutex> lock(sync_->mutex_);
    while (Q_.empty()) sync_->condvar_.wait(lock);
    T t = Q_.front();
    return t;
}

template <typename T>
bool BlockingQueue<T>::try_pop(T *t) {
    std::unique_lock<std::mutex> lock(sync_->mutex_);
    if (Q_.empty()) return false;
    *t = Q_.front();
    Q_.pop();
    return true;
}

template <typename T>
bool BlockingQueue<T>::try_peek(T *t) {
    std::unique_lock<std::mutex> lock(sync_->mutex_);
    if (Q_.empty()) return false;
    *t = Q_.front();
    return true;
}

template <typename T>
size_t BlockingQueue<T>::size() {
    std::unique_lock<std::mutex> lock(sync_->mutex_);
    return Q_.size();
}
#endif  // SRC_COMMON_THREAD_BLOCKINGQUEUE_H_
