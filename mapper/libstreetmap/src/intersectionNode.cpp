

#include "intersectionNode.h"
#include "common.h"
#include "streetSegment.h"
#include <algorithm>


//destructor
intersectionNode:: ~intersectionNode(){
}

double calcDistBetTwo(int inter1, int inter2){

    LatLon p1= intersectionDataBase[inter1]->getLatLon();
 LatLon p2= intersectionDataBase[inter2]->getLatLon();

 return calcDistBetTwo(p1, p2);
}

//returns street names attached to an intersection
 vector<string> intersectionNode:: getStreetVector(){
     return streetNames;
 }

 void intersectionNode:: setXY(pair<double,double> t){
     xy=t;
 }
 
 pair<double,double> intersectionNode:: getXY(){
     return xy;
 }
 
 
 //constructor
 intersectionNode:: intersectionNode(unsigned int _intersectionID){

    numOfStreetSegments = getIntersectionStreetSegmentCount(_intersectionID);
    intersectionName = getIntersectionName(_intersectionID);

    intersectionID = _intersectionID;
    position = getIntersectionPosition(_intersectionID);
    
    InterToSeg[_intersectionID].resize(numOfStreetSegments);
    streetNames.resize(numOfStreetSegments);
    latList[_intersectionID] = position.lat();
//    highLighted=false;
    for (int i = 0; i < numOfStreetSegments; i++) {

        
                auto segId= getIntersectionStreetSegment(_intersectionID, i);
        InterToSeg[intersectionID][i] =segId;
        string streetName = streetSegmentDataBase[ segId]->getStreetName();
        
        if(streetSegmentDataBase[segId]->otherInterIsAdjacent(_intersectionID)){
            pair<float, int> p (streetSegmentDataBase[ segId]->getSegTravelTime(),segId);
            graph[_intersectionID].adjacent.push_back(p);
        
        }
         if(streetSegmentDataBase[segId]->otherInterIsSource(_intersectionID)){
            pair<float, int> p (streetSegmentDataBase[ segId]->getSegTravelTime(),segId);
            graph[_intersectionID].source.push_back(p);
        
        }
        streetNames[i]=streetName;
        
        
    }
interByString[intersectionName].push_back(_intersectionID);
 }
 
LatLon intersectionNode:: getLatLon(){
      return position;
  }
 
string intersectionNode:: getName(){
    return intersectionName;
}

//returns number of street segments associated with a node
int intersectionNode:: getNumOfStreetSegments(){
      return numOfStreetSegments;
}
 
//returns the Ids of the street segments at the nodes
//vector<unsigned int> intersectionNode:: getAdjacentSegmentVector(){
//       return adjacentSegmentIDs;
//   }
//  
////returns segment ID
//unsigned int intersectionNode:: getSegmentID(int num){
//       return adjacentSegmentIDs[num];
//   }
   
