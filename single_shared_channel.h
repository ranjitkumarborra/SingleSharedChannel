//
//  single_shared_channel.h
//  SysDig
//
//  Created by Ranjit Borra on 6/24/18.
//  Copyright © 2018 Ranjit Borra. All rights reserved.
//

#ifndef single_shared_channel_h
#define single_shared_channel_h

#include <chrono>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <map>
#include <condition_variable>

#include "udp_client.h"
#include "sync_queue.h"

using namespace std;
using namespace chrono;
namespace sysdig {
    
    class SingleSharedChannel{
    public:
        static SingleSharedChannel * getSingleSharedChannel(string ip,
                                                            int port,
                                                            int message_rate,
                                                            int queue_size);
        static bool  destroySingleSharedChannel(string ip, int port);
        static mutex create_lock_;
        static map<string, SingleSharedChannel *> channel_lookup_;
        bool isActive() const;
        bool sendMessage(char * message, size_t msg_size);
        SingleSharedChannel(const SingleSharedChannel &) = delete;
        SingleSharedChannel& operator =(const SingleSharedChannel &) = delete;
    private:
        SingleSharedChannel(string ip,
                            int port,
                            int message_rate,
                            int queue_size);
        
        ~SingleSharedChannel();
        
        void monitorMessageRate(void);
        void destroyChannel();
        void monitorUdpBuffer();
        //Data Members ...
        atomic<bool> destroy_channel_;
        atomic<bool> push_to_queue_;
        atomic<int>  counter_;
        const int message_rate_;
        const string ip_;
        const int port_;
        std::thread * timer_thread_;
        std::thread * queue_processor_thread_;
        decltype(std::chrono::high_resolution_clock::now()) last_received_;
        decltype(last_received_) last_reset_;
        condition_variable cv_;
        RingBuffer<char> ring_buffer_;
        UdpClient udp_client_;
    };
    
    map<string, SingleSharedChannel *> SingleSharedChannel::channel_lookup_;
    mutex SingleSharedChannel::create_lock_;
    
    SingleSharedChannel * SingleSharedChannel::getSingleSharedChannel(string ip,
                                                                      int port,
                                                                      int message_rate,
                                                                      int queue_size){
        auto channel_address = ip + to_string(port);
        if(channel_lookup_.find(channel_address) != channel_lookup_.end())
            return channel_lookup_[channel_address];
        unique_lock<mutex> lck(create_lock_);
        if(channel_lookup_.find(channel_address) != channel_lookup_.end())
            return channel_lookup_[channel_address];
        else{
            auto new_channel = new SingleSharedChannel(ip, port, message_rate, queue_size);
            channel_lookup_[channel_address] = new_channel;
            return new_channel;
        }
    }
    
    SingleSharedChannel::SingleSharedChannel(string ip,
                                             int port,
                                             int message_rate,
                                             int queue_size): port_(port),
                                             message_rate_(message_rate),
                                             ring_buffer_(queue_size),
                                             udp_client_(ip, port){
                                        
         timer_thread_ = new std::thread(&SingleSharedChannel::monitorMessageRate, this);
         queue_processor_thread_ = new std::thread(&SingleSharedChannel::monitorUdpBuffer, this);
        
    }
    SingleSharedChannel::~SingleSharedChannel(){
        delete timer_thread_;
        delete queue_processor_thread_;
    }
    bool SingleSharedChannel::sendMessage(char *msg, size_t msg_size){
        counter_ ++;
        last_received_ = high_resolution_clock::now();
        if(push_to_queue_) {
            auto status = ring_buffer_.write(msg);
            if(status <0){
                delete msg;
            }
        }
        else {
            udp_client_.send(msg);
        }
        return true;
    }
    void SingleSharedChannel::destroyChannel(){
        destroy_channel_ = true;
        timer_thread_->join();
        queue_processor_thread_->join();
        delete this;    //no instruction after deleting current object ...
    }
    
    bool SingleSharedChannel::isActive() const{
        return !destroy_channel_;
    }
    
    void SingleSharedChannel::monitorMessageRate(void){
        mutex m;
        unique_lock<mutex> lck;
        last_reset_ = high_resolution_clock::now();
        while (!destroy_channel_) {
            cv_.wait(lck, [&](){ return counter_ == message_rate_;});
            if( (last_received_ - last_reset_).duration::count() <= 1 || !ring_buffer_.isEmpty()) push_to_queue_ = true;
            else push_to_queue_ = false;
            counter_ = 0;
            last_reset_ = high_resolution_clock::now();
        }
    }
    void SingleSharedChannel::monitorUdpBuffer(){
        mutex m;
        unique_lock<mutex> lck;
        while (!destroy_channel_) {
            cv_.wait(lck, [&](){ return !ring_buffer_.isEmpty() || destroy_channel_;});
            char* send_buffer = ring_buffer_.read();
            if(send_buffer)
                udp_client_.send(send_buffer);
        }
    }
    
}   //namespace sysdig ...

#endif /* single_shared_channel_h */
