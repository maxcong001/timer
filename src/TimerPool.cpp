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




