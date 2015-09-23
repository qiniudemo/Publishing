#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

#ifdef __STD_THREAD_SUPPORT__

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

template <class T>
class PacketQueue
{
public:
        PacketQueue();
        T Pop();
        bool TryPop(T &item);
        void Push(const T &item);
        void Push(T &&item);

        // though I do not recommend using empty() or size() in concurrent environment
        // but it is natural to use it for debugging or verbose outputs purpose
        bool IsEmpty();
        size_t Size();
private:
        queue<T> m_queue;
        mutex m_mutex;
        condition_variable m_condition;
};

template <class T>
PacketQueue<T>::PacketQueue():
        m_queue(queue<T>()),
        m_mutex(),
        m_condition()
{}

template <class T>
T PacketQueue<T>::Pop()
{
        unique_lock<mutex> mutexLock(m_mutex);
        while (m_queue.empty()) {
                m_condition.wait(mutexLock);
        }
        auto item = m_queue.front();
        m_queue.pop();
        return item;
}

template <class T>
bool PacketQueue<T>::TryPop(T &_item)
{
        unique_lock<mutex> mutexLock(m_mutex);
        if (! m_queue.empty()) {
                _item = m_queue.front();
                m_queue.pop();
                return true;
        }
        return false;
}

template <class T>
void PacketQueue<T>::Push(const T &_item)
{
        unique_lock<mutex> mutexLock(m_mutex);
        m_queue.push(_item);
        mutexLock.unlock();
        m_condition.notify_one();
}

template <class T>
void PacketQueue<T>::Push(T &&_item)
{
        unique_lock<mutex> mutexLock(m_mutex);
        m_queue.push(move(_item));
        mutexLock.unlock();
        m_condition.notify_one();
}

template <class T>
bool PacketQueue<T>::IsEmpty()
{
        lock_guard<mutex> mutexLock(m_mutex);
        return m_queue.empty();
}

template <class T>
size_t PacketQueue<T>::Size()
{
        lock_guard<mutex> mutexLock(m_mutex);
        return m_queue.size();
}

#endif // __STD_THREAD_SUPPORT__

#endif
