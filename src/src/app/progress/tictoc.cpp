 #include "app/progress/tictoc.h"

 void Timer::tic() {
    tstart = TClock::now();
}

double Timer::toc() {
    auto tend = TClock::now();

    auto period = TClock::period();
    double dt = static_cast<double>(period.num)/period.den;
    double t = (tend - tstart).count()*dt;
    return t;
}
 

Timer& getGlobalTimer() {
    static Timer timer;
    return timer;
}

void tic() {
    getGlobalTimer().tic();
}
double toc() {
    return getGlobalTimer().toc();
}
