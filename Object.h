#ifndef _OBJECT_H
#define _OBJECT_H
#include "Vect.h"
#include "Color.h"
#include "Ray.h"
class Object{

    public:
    virtual Color getColor()=0;
    virtual double findIntersection(Ray ray)=0;
    virtual Vect getNormalAt(Vect point)=0;
};


#endif