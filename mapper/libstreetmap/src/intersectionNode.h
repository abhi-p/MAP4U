/* 
 * File:   intersectionNode.h
 * Author: sampanth
 *
 * Created on January 24, 2017, 3:39 PM
 */

#ifndef INTERSECTIONNODE_H
#define INTERSECTIONNODE_H
# include <string>
#include "StreetsDatabaseAPI.h"
//#include "common.h"

using namespace std;
double calcDistBetTwo(int inter1, int inter2);
//class for all info surrounding intersections
class intersectionNode{
    public:
    
        ~intersectionNode();
        intersectionNode(unsigned int _intersectionID);
        intersectionNode* insert(LatLon pos, double dist, unsigned int index);
        int getNumOfStreetSegments();
        string getName();
        LatLon getLatLon();
        void setXY(pair<double,double> t);
        pair<double,double> getXY();
//        void setHighlightedTo(bool on);
//          bool isHighlighted();
        intersectionNode(float dist, unsigned int _intersectionID);
//        vector<unsigned int> getAdjacentSegmentVector();
        vector<string> getStreetVector();
    
    private:
        string intersectionName;
        unsigned int intersectionID;
//        bool highLighted;
        vector<string> streetNames;
        int numOfStreetSegments;
        LatLon position;
        pair<double,double> xy;
    

};

#endif /* INTERSECTIONNODE_H */

