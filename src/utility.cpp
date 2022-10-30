#include "utility.h"


bool processQEvents(double intervalSec)
{
    using namespace std::chrono;

    static high_resolution_clock::time_point startTime = high_resolution_clock::now();
    static high_resolution_clock::time_point endTime;


    endTime = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(endTime - startTime);
    if(time_span.count() >= intervalSec)
    {
        startTime = endTime;
        QApplication::processEvents();
        return true;
    }
    return false;
}
