#ifndef _PLANE_H
#define _PLANE_H
#include "Vect.h"
#include "Color.h"
#include "math.h"
#include "Object.h"
class Plane : public Object{
    Vect normal;
    double distance;
    Color color;

    public:
    Plane();
    Plane(Vect,double,Color);
    Vect getPlaneNormal(){return normal;}
    double getPlaneDistance(){return distance;}
    Color getColor(){return color;}
    Vect getNormalAt(Vect point){
        return normal;
    }

    double findIntersection(Ray ray){
        // for full details, visit http://www.ambrsoft.com/TrigoCalc/Plan3D/PlaneLineIntersection_.htm
        Vect rayDirection = ray.getRayDirection();
        double a = rayDirection.dotProduct(normal);
        //if ray parallel to plane
        if(a==0){
            return -1;
        }
        else{
            double b = normal.dotProduct(ray.getRayOrigin().vectAdd(normal.vectMultiply(distance).negative()));
            return -b/a;
        }
        
    }
};

Plane::Plane(){
    normal = Vect(1,0,0);
    distance = 0;
    color = Color(0.5,0.5,0.5,0);
}

Plane::Plane(Vect normalValue, double distanceValue, Color colorValue){
    normal = normalValue;
    distance = distanceValue;
    color = colorValue;
}

#endif