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