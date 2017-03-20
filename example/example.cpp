#include "Timer.hpp"
using namespace std;
/**************************************************************************/
// Test
void timer_proc(void *args) {
    cout << args << endl;
}

int main() {
    list<Timer*> l;
    for(int i=0; i<1000;++i) {
        Timer *t = new Timer();
        t->start(500, timer_proc, reinterpret_cast<void*>(i));
        l.push_back(t);
    }

    sleep(3);

    for(Timer* todel:l)
    {
        delete(todel);
    }

    return 0;
}
