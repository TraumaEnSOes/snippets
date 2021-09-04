#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template< typename T, typename CONTAINER = std::queue< T > > class Queue {
    mutable std::mutex m_mutex;
    CONTAINER m_queue;
    std::condition_variable m_cond;

public:
    T dequeue( ) {
        std::unique_lock< std::mutex > mlock( m_mutex );

        while( m_queue.empty( ) ) {
            m_cond.wait( mlock );
        }

        T item( std::move( m_queue.front( ) ) );
        m_queue.pop( );

        return item;
    }

    void enqueue( T item ) {
        std::unique_lock< std::mutex > mlock( m_mutex );

        m_queue.push( std::move( item ) );
        mlock.unlock( );
        m_cond.notify_one( );
    }

    typename CONTAINER::size_type size( ) const {
        std::unique_lock< std::mutex > mlock( m_mutex );

        return m_queue.size( );
    }

    void clear( ) {
        std::unique_lock< std::mutex > mlock( m_mutex );

        m_queue.clear( );
    }
};

#endif
