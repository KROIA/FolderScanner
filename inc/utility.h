#ifndef UTILITY_H
#define UTILITY_H

#include <ctime>
#include <ratio>
#include <chrono>
#include <QApplication>

// returns true if QApplication::processEvents() was executed
extern bool processQEvents(double intervalSec = 0.1);

#endif // UTILITY_H
