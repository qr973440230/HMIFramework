#ifndef ABSTRACTQUEUE_H
#define ABSTRACTQUEUE_H

/**
 * multi thread read and write queue
 * 1.write
 *  1) call peekWriteable function acquire buffer, if return nullptr, get buffer failure!
 *  2) call push function finish your write, if peekWriteable return nullptr, not call push function.
 * 2.read
 *  1) call peekReadable function acquire buffer, if return nullptr, get buffer failure!
 *  2) call next function finish your read, if peekReadable return nullptr, not call next function.
 * 3.abort
 *  1) call abort function abort your queue, call isAbort function check queue is abort.
 * Warning!!!
 * 1.if queue is abort, peekWriteable will return nullptr.
 * 2.if queue is abort or read timeout, peekReadable will return nullptr.
 */
template <typename T>
class AbstractQueue
{
public:
    AbstractQueue();
    virtual ~AbstractQueue();

    virtual T * peekReadable(unsigned long timeout) = 0;
    virtual void next(T * data) = 0;

    virtual T * peekWriteable() = 0;
    virtual void push(T * data) = 0;

    virtual void abort() = 0;
    virtual bool isAbort() = 0;
};

template<typename T>
AbstractQueue<T>::AbstractQueue()
{

}

template<typename T>
AbstractQueue<T>::~AbstractQueue()
{

}

#endif // ABSTRACTQUEUE_H
