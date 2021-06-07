#ifndef _SPHERE_H
#define _SPHERE_H
#include "Vect.h"
#include "Color.h"
#include "math.h"
#include "Object.h"
class Sphere : public Object{
    Vect center;
    double radius;
    Color color;

    public:
    Sphere();
    Sphere(Vect,double,Color);
    Vect getSphereCenter(){return center;}
    double getSphereRadius(){return radius;}
    Color getColor(){return color;}

    Vect getNormalAt(Vect point){
        // normal always points away from the center of the speher
        Vect normal = point.vectAdd(center.negative()).normalize();
        return normal;
    }

    double findIntersection(Ray ray){
        //for full details, visit http://www.ambrsoft.com/TrigoCalc/Sphere/SpherLineIntersection_.htm
        Vect rayOrigin = ray.getRayOrigin();
        double rayOriginX = rayOrigin.getVectX();
        double rayOriginY = rayOrigin.getVectY();
        double rayOriginZ = rayOrigin.getVectZ();

        Vect rayDirection = ray.getRayDirection();
        double rayDirectionX = rayDirection.getVectX();
        double rayDirectionY = rayDirection.getVectY();
        double rayDirectionZ = rayDirection.getVectZ();

        double sphereCenterX = center.getVectX();
        double sphereCenterY = center.getVectY();
        double sphereCenterZ = center.getVectZ();

        // will equal 1 since ray direction is normalized. Therefore can use a=1 instead
        double a=pow(rayDirectionX,2) + pow(rayDirectionY,2) + pow(rayDirectionZ,2);

        double b = 2*(rayOriginX-sphereCenterX)*rayDirectionX + 2*(rayOriginY-sphereCenterY)*rayDirectionY + 2*(rayOriginZ-sphereCenterZ)*rayDirectionZ;
        double c = pow(rayOriginX-sphereCenterX,2) + pow(rayOriginY-sphereCenterY,2) + pow(rayOriginZ-sphereCenterZ,2) - radius*radius;
        double d = b*b - 4*a*c;
        
        if(d>0){
            //root exists, hence intersection exists. Thus two roots/points of intersections exist. Hence the smaller one
            //is the intersection we need.
            double root1 = (-b - sqrt(d))/(2*a);
            if(root1>0){
                return root1;
            }else{
                double root2 = (-b + sqrt(d))/(2*a);
                return root2;
            }

        } else{
            //ray missed the sphere
            return -1;
        }
    }
};

Sphere::Sphere(){
    center = Vect(0,0,0);
    radius = 1.0;
    color = Color(0.5,0.5,0.5,0);
}

Sphere::Sphere(Vect centerValue, double radiusValue, Color colorValue){
    radius = radiusValue;
    center = centerValue;
    color = colorValue;
}

#endif