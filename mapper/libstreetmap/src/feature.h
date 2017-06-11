/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   feature.h
 * Author: sampanth
 *
 * Created on February 14, 2017, 3:21 PM
 */

#ifndef FEATURE_H
#define FEATURE_H
# include <string>
#include "StreetsDatabaseAPI.h"
# include "graphics.h"

using namespace std;

class feature{
public:
    feature(unsigned int index);
    vector<double> getX();
    vector<double> getY();
    t_point* getPointsPointer();
     t_point getPoint(int index);
    string getColour();
    int getNumOfPoints();
    string decideColour(FeatureType type);
    t_point getCenterOfPoly();
      t_point  getCenter();
    ~feature();
private:
    string name;
    t_point* featurePoints;
    int numOfPoints;
    bool isPolygon;
    double area;
    string colour;
  t_point center;
    
    
    
    
};






#endif /* FEATURE_H */

