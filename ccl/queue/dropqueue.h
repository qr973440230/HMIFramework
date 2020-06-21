#ifndef DROPQUEUE_H
#define DROPQUEUE_H

#include "abstractqueue.h"
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#define DROP_DEFAULT_QUEUE_MAX_SIZE 20
#define DROP_DEFAULT_TIME_OUT 500

template <typename T>
struct DropNode{
    T data;
    DropNode<T> * next;
    DropNode<T> * pre;
};

/**
 * if buffer overflow, will drop oldest data.
 */
template <typename T>
class DropQueue: public AbstractQueue<T>{

public:
    explicit DropQueue();
    explicit DropQueue(unsigned int maxSize,unsigned long dropTimeout);

    virtual T * peekReadable(unsigned long timeout) override;
    virtual void next(T * data) override;

    virtual T * peekWriteable() override;
    virtual void push(T * data) override;

    virtual void abort() override;
    virtual bool isAbort() override;

private:
    DropNode<T> * m_wIdx;
    DropNode<T> * m_rIdx;

    unsigned int m_maxSize;
    unsigned long m_dropTimeout;
    bool m_abort;

    QMap<T*,DropNode<T>*> m_map;
    QMutex m_mutex;
    QWaitCondition m_cond;
};

template<typename T>
DropQueue<T>::DropQueue()
    :m_wIdx(nullptr),
      m_rIdx(nullptr),
      m_maxSize(DROP_DEFAULT_QUEUE_MAX_SIZE),
      m_dropTimeout(DROP_DEFAULT_TIME_OUT),
      m_abort(false)
{
    m_wIdx = new DropNode<T>();
    m_rIdx = new DropNode<T>();
    m_wIdx->pre = m_rIdx;
    m_wIdx->next = m_rIdx;
    m_rIdx->next = m_wIdx;
    m_rIdx->pre = m_wIdx;

    DropNode<T> * node = nullptr;
    for(unsigned int i = 0;i< m_maxSize;i++){
        node = new DropNode<T>();
        node->pre = m_wIdx;
        node->next = m_wIdx->next;
        node->pre->next = node;
        node->next->pre = node;

        m_map.insert(&node->data,node);
    }
}

template<typename T>
DropQueue<T>::DropQueue(unsigned int maxSize, unsigned long dropTimeout)
    :m_wIdx(nullptr),
      m_rIdx(nullptr),
      m_maxSize(maxSize),
      m_dropTimeout(dropTimeout),
      m_abort(false)
{
    m_wIdx = new DropNode<T>();
    m_rIdx = new DropNode<T>();
    m_wIdx->pre = m_rIdx;
    m_wIdx->next = m_rIdx;
    m_rIdx->next = m_wIdx;
    m_rIdx->pre = m_wIdx;

    DropNode<T> * node = nullptr;
    for(int i = 0;i< m_maxSize;i++){
        node = new DropNode<T>();
        node->pre = m_wIdx;
        node->next = m_wIdx->next;
        node->pre->next = node;
        node->next->pre = node;

        m_map.insert(&node->data,node);
    }
}

template<typename T>
T *DropQueue<T>::peekReadable(unsigned long timeout)
{
    QMutexLocker locker(&m_mutex);
    while(m_rIdx->next == m_wIdx && !m_abort){
        if(!m_cond.wait(&m_mutex,timeout)){
            // timeout
            return nullptr;
        }
    }

    if(m_abort){
        return nullptr;
    }

    // detach read node
    DropNode<T> *readNode = m_rIdx->next;
    readNode->pre->next = readNode->next;
    readNode->next->pre = readNode->pre;
    readNode->next = nullptr;
    readNode->pre = nullptr;

    return &readNode->data;
}

template<typename T>
void DropQueue<T>::next(T *data)
{
    if(!m_map.contains(data)){
        return;
    }

    m_mutex.lock();

    // insert read node
    DropNode<T> *readNode = m_map[data];
    readNode->pre = m_rIdx->pre;
    readNode->next = m_rIdx;
    readNode->pre->next = readNode;
    readNode->next->pre = readNode;
    readNode = nullptr;

    m_cond.wakeOne();
    m_mutex.unlock();
}

template<typename T>
T *DropQueue<T>::peekWriteable()
{
    QMutexLocker locker(&m_mutex);
    while(m_wIdx->next == m_rIdx && !m_abort){
        if(!m_cond.wait(&m_mutex,m_dropTimeout)){
            // timeout
            m_rIdx = m_rIdx->next;
        }
    }

    if(m_abort){
        return nullptr;
    }

    // detach write node
    DropNode<T> * writeNode = m_wIdx->next;
    writeNode->pre->next = writeNode->next;
    writeNode->next->pre = writeNode->pre;
    writeNode->pre = nullptr;
    writeNode->next = nullptr;

    return &writeNode->data;
}

template<typename T>
void DropQueue<T>::push(T *data)
{
    if(!m_map.contains(data)){
        return;
    }

    m_mutex.lock();

    // insert write node
    DropNode<T> * writeNode = m_map[data];
    writeNode->pre = m_wIdx->pre;
    writeNode->next = m_wIdx;
    writeNode->pre->next = writeNode;
    writeNode->next->pre = writeNode;
    writeNode = nullptr;

    m_cond.wakeOne();
    m_mutex.unlock();
}

template<typename T>
void DropQueue<T>::abort()
{
    m_mutex.lock();
    m_abort = true;
    m_cond.wakeAll();
    m_mutex.unlock();
}

template<typename T>
bool DropQueue<T>::isAbort()
{
    QMutexLocker locker(&m_mutex);
    return m_abort;
}

#endif // DROPQUEUE_H
