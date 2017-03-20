/*
 * Copyright (c) 2016-20017 Max Cong <savagecm@qq.com>
 * this code can be found at https://github.com/maxcong001/timer
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "Timer.hpp"

using namespace std;


static TimerPool timer_pool_glob;
std::mutex timer_mutex;
TimerPool::TimerPool() {
    try {

        // Create epoll
        m_epoll_fd = epoll_create(MaxEPOLLSize);
        if(m_epoll_fd == -1) {
            perror("epoll_create");
            throw;
        }

        // Create thread for epoll
        int res = pthread_create(&m_tid, 0, TimerPool::epoll_proc, 0);
        if(res == -1) {
            perror("pthread_create");
            throw;
        }
    } catch (...) {}
}

void* TimerPool::epoll_proc(void *) {
    struct epoll_event events[MaxEPOLLSize];
    while(1) {
        // Wait for notice
        int n =epoll_wait(timer_pool_glob.m_epoll_fd, events, MaxEPOLLSize, -1);
        for(int i=0; i<n; ++i) {
            int fd = events[i].data.fd;
            // Clear buffer
            uint64_t buf;
            read(fd, &buf, sizeof(uint64_t));

            // Call the callback function when timer expiration
            Timer::TimerEvent te = TimerPool::get_timer_event(events[i].data.fd);
            if(te.cbf) {
                te.cbf(te.args);
            }
        }
    }
    return 0;
}

Timer::TimerEvent TimerPool::get_timer_event(int fd) {
    std::lock_guard<std::mutex> lock(timer_mutex);
    return timer_pool_glob.m_map_te[fd];
}

bool TimerPool::add_timer_event(const Timer::TimerEvent &te) {
    // Add timer event for epoll
    struct epoll_event epe;
    epe.data.fd = te.fd;
    epe.events = EPOLLIN | EPOLLET;
    int res = epoll_ctl(timer_pool_glob.m_epoll_fd, EPOLL_CTL_ADD, te.fd, &epe);
    if(res == -1) {
        perror("epoll_ctl");
        return false;
    }
    std::lock_guard<std::mutex> lock(timer_mutex);
    // Insert timer event to map
    timer_pool_glob.m_map_te[te.fd] = te;

    return true;
}

void TimerPool::remove_timer_event(const int fd) {
    // Remove from epoll
    int res = epoll_ctl(timer_pool_glob.m_epoll_fd, EPOLL_CTL_DEL, fd,0);
    if(res == -1) {
        perror("epoll_ctl");
        return;
    }

    // Remove from map
    std::lock_guard<std::mutex> lock(timer_mutex);
    MapTimerEvent::iterator iter = timer_pool_glob.m_map_te.find(fd);
    timer_pool_glob.m_map_te.erase(iter);
}




