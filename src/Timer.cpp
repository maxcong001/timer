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

bool Timer::start(const uint interval, CALLBACK_FN cbf, void *args, const bool triggered_on_start) {
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

        // Add timer for epoll
        TimerEvent te;
        te.fd = fd;
        te.cbf = cbf;
        te.args = args;
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

