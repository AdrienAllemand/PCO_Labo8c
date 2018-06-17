#ifndef RUNNABLE_H
#define RUNNABLE_H

// CORE: compilation error here #include "QString.h"-> #include <QString>
#include "QString.h"

class Runnable {
public:
    Runnable(){}
    virtual ~Runnable() {}
    virtual void run() = 0;
    virtual QString id() = 0;
};

#endif // RUNNABLE_H
