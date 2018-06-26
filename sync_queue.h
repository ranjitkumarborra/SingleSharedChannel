//
//  sync_queue.h
//  SysDig
//
//  Created by Ranjit Borra on 6/23/18.
//  Copyright © 2018 Ranjit Borra. All rights reserved.
//

#ifndef sync_queue_h
#define sync_queue_h

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <array>

using namespace std;
namespace sysdig {
    /*
     * RingBuffer is a multi producer and single consumer queue
     * with out locks.
     */
    template <class T, int  CAPACITY = 100>
    class RingBuffer {
        typedef T value_type;
    public:
        
        RingBuffer& operator = (const RingBuffer&) = delete;
        
        // size must be >= 2.
        //
        // Also, note that the number of usable slots in the queue at any
        // given time is actually (size-1), so if you start with an empty queue,
        // isFull() will return true after size-1 insertions.
        explicit RingBuffer(uint32_t size)
        : size_(size)
        , read_index_(0)
        , write_index_(0)
        {
            //records_.resize(size_, nullptr);
            assert(size >= 2);
        }
        
        ~RingBuffer() {
            // We need to destruct anything that may still exist in our queue.
            // (No real synchronization needed at destructor time: only one
            // thread can be doing this.)
            if (!std::is_trivially_destructible<T>::value) {
                size_t read_index = read_index_;
                size_t endIndex = write_index_;
                while (read_index != endIndex) {
                    delete records_[read_index];
                    if (++read_index == size_) {
                        read_index = 0;
                    }
                }
            }
        }
        
        
        int write(T * rec_to_insert) {
            auto rec = rec_to_insert;
            int next_record = -1;
            while(1){
                if(isFull()) return -1;
                else {
                    //get the token...
                    auto const current_write = write_index_.load(std::memory_order_acquire); //strict reading and writing...
                    next_record = current_write + 1;
                    
                    if (next_record == size_) {
                        next_record = 0;
                    }
                    if (next_record != read_index_.load(std::memory_order_acquire)) {
                        //try to write new record ...
                        auto to_location = &records_[next_record];
                        if(atomic_exchange(to_location, rec) == nullptr){      //rec is local, consistency is guranteed...
                            write_index_.store(next_record, std::memory_order_acquire);
                            return next_record;
                        }
                        else rec = rec_to_insert;
                    }
                }
            }
        }
        
        T* read() {
            auto const current_read = read_index_.load(std::memory_order_relaxed);
            if (current_read == write_index_.load(std::memory_order_acquire)) {
                // queue is empty
                return nullptr;
            }
            auto next_record = current_read + 1;
            if (next_record == size_) {
                next_record = 0;
            }
            T* ret = records_[current_read];
            records_[current_read] = nullptr;
            read_index_.store(next_record, std::memory_order_release);
            return ret;
        }
        
        bool isEmpty() const {
            return read_index_.load(std::memory_order_acquire) ==
            write_index_.load(std::memory_order_acquire);
        }
        
        bool isFull() const {
            auto next_record = write_index_.load(std::memory_order_acquire) + 1;
            if (next_record == size_) {
                next_record = 0;
            }
            if (next_record != read_index_.load(std::memory_order_acquire)) {
                return false;
            }
            // queue is full
            return true;
        }
        
        // maximum number of items in the queue.
        size_t capacity() const {
            return size_ - 1;
        }
        
    private:
        using AtomicIndex = std::atomic<unsigned int>;
        const uint32_t size_ = CAPACITY; //TODO::change this as template parameter ...
        std::array<atomic<T*>, CAPACITY> records_ = {};  //TODO::change it to void *.. to deal with size...
        AtomicIndex read_index_;
        AtomicIndex write_index_;
        
    };
    
} // namespace sysdig
#endif /* sync_queue_h */
