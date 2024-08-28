// DrawThread.h
#ifndef DRAW_THREAD_H
#define DRAW_THREAD_H

#include "CommonObjects.h"

class DrawThread {
public:
    void operator()(CommonObjects& common);
};

#endif // DRAW_THREAD_H
