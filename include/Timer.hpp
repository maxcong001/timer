#include <cstdio>  
#include <cstdlib>  
#include <cstring>  
#include <cmath>  
#include <unistd.h>  
#include <sys/timerfd.h>  
#include <sys/epoll.h>  
#include <sys/types.h>  
#include <stdint.h>  
#include <pthread.h>  
#include <iostream>  
#include <map>  
#include <algorithm>  
#include <list>  
#include <mutex>


class Timer {  
public:  
    Timer();  
    ~Timer();  
      
    // The structure of timer event  
    typedef void (*CALLBACK_FN)(void *);  
    typedef struct _TimerEvent {  
        int fd;  
        CALLBACK_FN cbf;  
        void *args;  
    } TimerEvent;    
  
    /* 
     *  Name: start 
     *  Brief: start the timer 
     *  @interval: The interval, the unit is ms 
     *  @cbf: The callback function 
     *  @args: The arguments of callback function 
     *  @triggered_on_start: Determine tirggered on start or not 
     * 
     */   
    bool start(const uint interval, CALLBACK_FN cbf,  
            void *args,const bool triggered_on_start=false);  
  
    /* 
     *  Name: stop 
     *  Brief: stop the timer 
     * 
     */  
    void stop();  
  
private:  
    bool m_is_start;  
    TimerEvent m_te;  
};  

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
        typedef std::map<int, Timer::TimerEvent> MapTimerEvent;
        MapTimerEvent m_map_te;
        pthread_t m_tid;
        
};


