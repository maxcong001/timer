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




Timer::Timer() : m_is_start(false) {
    ::memset(&m_te, 0, sizeof(TimerEvent));
}

Timer::~Timer() {
    if(m_is_start) {
        stop();
        m_is_start = false;
    }
}

bool Timer::start(const uint interval, CALLBACK_FN cbf, void *args, const bool triggered_on_start, int* timer_fd) {
    if(!m_is_start) {
        if(!cbf) {
            cout << "start:" << "callback function can't set to be null" << endl;
            return false;
        }

        // Create timer
        struct itimerspec timer;
        double dfsec = (double)interval/1000;
        uint32_t sec=dfsec;
        uint64_t number_ns = 1000000000;
        uint64_t nsec = dfsec>=1 ? fmod(dfsec,(int)dfsec)*number_ns : dfsec*number_ns;
        timer.it_value.tv_nsec = triggered_on_start ? 0 : nsec;
        timer.it_value.tv_sec = triggered_on_start ? 0 : sec;
        timer.it_interval.tv_nsec = nsec;
        timer.it_interval.tv_sec = sec;

        int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        if(fd == -1) {
            perror("timerfd_create");
            return false;
        }

        int res = timerfd_settime(fd, 0, &timer, 0);
        if(res == -1) {
            perror("timerfd_settime");
            return false;
        }
        if (!timer_fd)
        {
            *timer_fd = fd;
        }

        // Add timer for epoll
        TimerEvent te;
        te.fd = fd;
        te.cbf = cbf;
        te.args = args;
        te.timer_instance = this;
        res = TimerPool::add_timer_event(te);
        if(res == false) {
            return false;
        }

        // Change the attributes of class
        m_te = te;
        m_is_start = true;
    } else {
        cout << "start:Timer already start" << endl;
        return false;
    }

    return true;
}

void Timer::stop() {
 //   std::lock_guard<std::mutex> lock(timer_mutex);

    // Remove from map and epoll
    TimerPool::remove_timer_event(m_te.fd);

    // Close the timer
    int res = close(m_te.fd);
    if(res == -1) {
        perror("close");
    }

    // Clear the attributes of class
    m_is_start = false;

}
void  Timer::stop(int timer_fd)
{
    Timer::TimerEvent te = TimerPool::get_timer_event(timer_fd);
    if (!te.timer_instance)
    {
        delete te.timer_instance;
    }
}

