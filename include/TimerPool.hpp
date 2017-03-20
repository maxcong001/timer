#include "Timer.hpp"
  
using namespace std;  


class TimerPool {
    public:
        TimerPool();
        ~TimerPool() {
        }

        // Some constant
        enum {
            MaxEPOLLSize = 20000,    
        };

        /*
         *  Name: epoll_proc
         *  Brief: this function run on new thread for epoll
         *
         */
        static void* epoll_proc(void *);

        /*
         *  Get the timer event by fd
         *
         */
        static Timer::TimerEvent get_timer_event(int fd);

        /*
         *  Add the timer event to map and epoll
         *
         */ 
        static bool add_timer_event(const Timer::TimerEvent &te); 

        /*
         *  Remove the timer event from map adn epoll by fd
         *
         */
        static void remove_timer_event(const int fd);

        // Map of file descriptor
        int m_epoll_fd;
        typedef map<int, Timer::TimerEvent> MapTimerEvent;
        MapTimerEvent m_map_te;
        pthread_t m_tid;
};

std::mutex timer_mutex;
static TimerPool timer_pool_glob;