#include "m1.h"

#include "StreetsDatabaseAPI.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <utility>
#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include "OSMDatabaseAPI.h"


#include "common.h"
//test
using namespace std;
string mapArgument;
streetSegment** streetSegmentDataBase;
street** streetDataBase;
intersectionNode** intersectionDataBase;
feature** featureDataBase;
Poi** poiDataBase;
unordered_map <string, vector<unsigned>> NameToStreetID;
unordered_map <string, vector<unsigned>> streetTypeToID;
unordered_map <string, vector<unsigned>> poiByString;
unordered_map <string, vector<unsigned>> interByString;
unordered_map <string, vector<unsigned>> streetNameToInterID;
unordered_map<string,bgi::rtree< value, bgi::quadratic<numberOfIterationbeforeOverFlow> > > generalRtree;
vector<pair<double, unsigned int>> featuresByArea;
vector<vector<unsigned int> > InterToSeg;
vector<double> latList;
 vector<double> xList;
vector<double> yList;
 vector<mapNode> graph;
vector<unsigned int> featuresWithNoArea;
float maxSpeed;
vector<const char*> autoComplete;
double avgLatInter;
//double avgLatPOI; 
 bool alreadyLoaded = false;
vector<double> segTravelTime;
vector<double> segLength;

double calcDistBetTwoRelative(point posOne, point posTwo);
 void parseMapPath(string mapPath);
void parseMapPath(string mapPath){
    int thirdSlash=26;
    string dot =".";
     int indexDot= mapPath.find(".");
    mapArgument=  mapPath.substr (thirdSlash,indexDot-thirdSlash);
      
    
}
 
 //calculate distance between two points (FOR POINT OF INTEREST FUNCTION)
 double calcDistBetTwoRelative(point posOne, point posTwo){

    //This simply gets the the point (x,y) points of two places and calculates
    //an inaccurate distance. This is used by the spatial functions as the
    //x and y points are already given, and an accurate distance is not required
    //Only the general distance is required. 
   
    double posOneLat=bg::get<0>(posOne);
    double posTwoLat= bg::get<0>(posTwo);
    double posOneLon= bg::get<1>(posOne);
    double posTwoLon= bg::get<1>(posTwo);
    //Does not have *EarhRadius of squrt to improve performance
  return pow(posTwoLat - posOneLat, 2) + pow(( posTwoLon - posOneLon), 2);
}
 
//loads map
bool load_map(std::string map_path) {
    
    //Load your map related data structures here

    if (!alreadyLoaded) {
        
            parseMapPath(map_path);
           // cout<<mapArgument<<endl;
        if (loadStreetsDatabaseBIN(map_path)&&loadOSMDatabaseBIN("/cad2/ece297s/public/maps/"+mapArgument+".osm.bin")) {
           
           //Simply defines the number of different features for use in a for loop
            unsigned int numOfStreets = getNumberOfStreets();
            unsigned int numOfStreetSegs = getNumberOfStreetSegments();
            unsigned int numOfIntersections = getNumberOfIntersections();
            unsigned int numOfPOIs = getNumberOfPointsOfInterest();
            unsigned int NumberOfWays=getNumberOfWays();
            
            unordered_map <OSMID, const OSMWay*> OSMIDToWay;

             
              for (unsigned int i=0;i<NumberOfWays;i++)
       {
           auto way=getWayByIndex(i);

           OSMID osmidOfWay=way->id();
          OSMIDToWay[osmidOfWay]=way;

           
       }
             
            streetDataBase = new street* [numOfStreets];
            maxSpeed=0;
           //Populates an unordered map with the street names and their
           //respective street ids
            for (unsigned int i = 0; i < numOfStreets; i++) {
                streetDataBase[i] = new street(i);
                NameToStreetID[streetDataBase[i]->retrieveStreetName()].push_back(i);
            }


            //Adding the street segments         

            streetSegmentDataBase = new streetSegment* [numOfStreetSegs];
            
            
            
            segLength.resize(numOfStreetSegs);
             segTravelTime.resize(numOfStreetSegs);
            for (unsigned int i = 0; i < numOfStreetSegs; i++) {
                streetSegmentDataBase[i] = new streetSegment( i);
                  OSMID osmidOfStreetSeg=streetSegmentDataBase[i]->getOSMId();
                   auto way= OSMIDToWay[osmidOfStreetSeg];
                   bool found=false;
                   int tagCount=getTagCount(way);
                   for(int j=0;j<tagCount&&!found;j++){
                   pair<string,string> h= getTagPair( way,j);
                   if (h.first=="highway")
                   {found=true;
                  streetSegmentDataBase[i]->assignType(h.second);
                   }

               }
                  
             
                int streetId = streetSegmentDataBase[i]->getStreetId();
                streetDataBase[streetId]->addStreetSegment(i);

                unsigned int intersectionFrom = streetSegmentDataBase[i]->getFrom();
                unsigned int intersectionTo = streetSegmentDataBase[i]->getTo();
                string streetName = streetDataBase[streetId]->retrieveStreetName();
                      
                streetDataBase[streetId]->addIntersection(intersectionFrom);
                streetDataBase[streetId]->addIntersection(intersectionTo);

                segTravelTime[i]=streetSegmentDataBase[i]->getSegTravelTime();
                segLength[i]=streetSegmentDataBase[i]->getSegLength();

            }
//Adding the intersections
             vector<string> tempAutoComplete;

            if (numOfIntersections > 0) {
                intersectionDataBase = new intersectionNode* [numOfIntersections];
                graph.resize(numOfIntersections);
                InterToSeg.resize(numOfIntersections);
                latList.resize(numOfIntersections);
                 xList.resize(numOfIntersections);
                  yList.resize(numOfIntersections);
                for (unsigned int i = 0; i < numOfIntersections; i++) {

                    intersectionDataBase [i] = new intersectionNode(i);
                    vector<string> streetNames = intersectionDataBase[i]->getStreetVector();
                    tempAutoComplete.push_back(intersectionDataBase[i]->getName());

                    int sizeOfVector = streetNames.size();
                    for (int j = 0; j < sizeOfVector; j++) {
                        streetNameToInterID[streetNames[j]].push_back(i);
                    }

                }
                //LatList is populated in the IntersectionNode constructor. It has
                //has the a list of all lats for the intersection. It is then sorted
                //so we can calculate the average latitude of the map
                sort(latList.begin(), latList.end());
                
                avgLatInter = (latList[0] + latList[numOfIntersections - 1]) / 2;
            }

            for (unsigned int i = 0; i < numOfIntersections; i++) {
                LatLon point1 = intersectionDataBase[i]->getLatLon();
                 xList[i]=earth_RADIUS_IN_METERS*point1.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD;
                         yList[i]=earth_RADIUS_IN_METERS*point1.lat() * DEG_TO_RAD;
                point IntersectionPosition = point(point1.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD, point1.lat() * DEG_TO_RAD);
                pair<double,double> p( xList[i],yList[i]);
                intersectionDataBase[i]->setXY(p);
                generalRtree["intersection"].insert(std::make_pair(IntersectionPosition, i));

            }
             sort(xList.begin(), xList.end());
              sort(yList.begin(), yList.end());
             
           //Doing the same thing as above to find the avg lat of the POI
            //vector<double>LatofPOI;
            //LatofPOI.resize(numOfPOIs);
            poiDataBase= new Poi*[numOfPOIs];
            for (unsigned int i = 0; i < numOfPOIs; i++) {
                        poiDataBase[i]= new Poi(i);
              tempAutoComplete.push_back(poiDataBase[i]->getName());

            }
            
            
            
            
            
 sort(tempAutoComplete.begin(), tempAutoComplete.end());
 tempAutoComplete.erase(unique(tempAutoComplete.begin(), tempAutoComplete.end()), tempAutoComplete.end());

 int size= tempAutoComplete.size();
 autoComplete.resize(size);
 
 
 for(int m = 0;m<size;m++){
 
  autoComplete[m]=tempAutoComplete[m].c_str();
 
 }
 
 

            for (unsigned int i = 0; i < numOfPOIs; i++) {

                LatLon point2 = getPointOfInterestPosition(i);
                point POIPosition = point(point2.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD, point2.lat() * DEG_TO_RAD);
                
                
                pair<double,double> p( earth_RADIUS_IN_METERS*point2.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD,earth_RADIUS_IN_METERS*point2.lat() * DEG_TO_RAD);
                poiDataBase[i]->setXY(p);
                 generalRtree["poi"].insert(std::make_pair(POIPosition, i));
                 generalRtree[poiDataBase[i]->getName()].insert(std::make_pair(POIPosition, i));
                  generalRtree[poiDataBase[i]->getType()].insert(std::make_pair(POIPosition, i));

            }
            unsigned int numOfFeatures = getNumberOfFeatures();
            featureDataBase= new feature* [numOfFeatures];
            for(unsigned int i=0;i<numOfFeatures;i++){
             featureDataBase[i]= new feature(i);  
             featureDataBase[i]->getCenterOfPoly();
            
            
            }
            sort(featuresByArea.begin(), featuresByArea.end());

           
            
            
            alreadyLoaded = true;
          
            return true;
        }

    }

    return false;
}

//closes map
void close_map() {

    //Clean-up your map related data structures here
    // alreadyLoaded=true;

    if (alreadyLoaded) {

        unsigned int numOfStreets = getNumberOfStreets();
        unsigned int numOfStreetSegs = getNumberOfStreetSegments();
        unsigned int numOfIntersections = getNumberOfIntersections();
         unsigned int numOfFeatures = getNumberOfFeatures();
           unsigned int numOfPoi = getNumberOfPointsOfInterest();
//Clears all data structures to a new map can be loaded
        for (unsigned int i = 0; i < numOfStreets; i++) {
            delete streetDataBase[i];
        }
        delete [] streetDataBase;

        for (unsigned int i = 0; i < numOfStreetSegs; i++) {
            delete streetSegmentDataBase[i];
        }
        delete[]streetSegmentDataBase;

        for (unsigned int i = 0; i < numOfIntersections; i++) {
            delete intersectionDataBase[i];
        }
        delete[]intersectionDataBase;
        
    for (unsigned int i = 0; i < numOfFeatures; i++) {
            delete featureDataBase[i];
        }
        delete[] featureDataBase;
          for (unsigned int i = 0; i < numOfPoi; i++) {
            delete poiDataBase[i];
        }
        delete[] poiDataBase;
        
        featuresByArea.clear();
        featuresWithNoArea.clear();
        NameToStreetID.clear();
        streetNameToInterID.clear();
        InterToSeg.clear();
        autoComplete.clear();
        segTravelTime.clear();
        segLength.clear();
        alreadyLoaded = false;
          generalRtree.clear();
     poiByString.clear();
        avgLatInter = 0;
        xList.clear();
        yList.clear();
         streetTypeToID.clear();
         graph.clear();
           closeStreetDatabase();
        closeOSMDatabase();
    }
   
}

//find street IDs from street name
vector<unsigned> find_street_ids_from_name(string street_name) {
  
    auto search=NameToStreetID.find(street_name);
    vector <unsigned int>empty;
    if(search != NameToStreetID.end()) {
    return NameToStreetID.find(street_name)->second;
    }
    else
    return NameToStreetID[street_name];
}

//find street segments making intersections
vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {

   
    return InterToSeg[intersection_id];
}

//find street names for intersections 
vector<std::string> find_intersection_street_names(unsigned intersection_id) {
   
    return intersectionDataBase[intersection_id]->getStreetVector();
}

//see if intersections are directly connected
bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
   
    if (intersection_id2 == intersection_id1) {
        return (true);
    }
    vector<unsigned int> streetSegs = InterToSeg[intersection_id1];
    int numOfStreetSegements = streetSegs.size();

    bool connected = false;
    for (int i = 0; !connected && i < numOfStreetSegements; i++) {
        connected = streetSegmentDataBase[streetSegs[i]]->matchingInterections(intersection_id1, intersection_id2);
    }
    return connected;
}

//find all intersections on street
vector<unsigned> find_all_street_intersections(unsigned street_id) {
//We get a vector of all the intersections on a street from StreetDatabase. Then we sort it and delete any duplicates before
//returning
    vector<unsigned> intersections = streetDataBase[street_id]->getIntersectionVector();
    sort(intersections.begin(), intersections.end());
    intersections.erase(unique(intersections.begin(), intersections.end()), intersections.end());
    return intersections;
}

//find neighbouring intersections
vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {

    vector<unsigned> adjacentIntersections;
    //get number of street segments attached to an intersection
    vector<unsigned int> streetSegs = InterToSeg[intersection_id];
    unsigned int numOfStreetSegments = streetSegs.size();

    for (unsigned int i = 0; i < numOfStreetSegments; i++) {

        if (streetSegmentDataBase[streetSegs[i]]->otherInterIsAdjacent(intersection_id)) {
            adjacentIntersections.push_back(streetSegmentDataBase[streetSegs[i]]->getOtherIntersectionID(intersection_id));
        }
    }
    //We get a vector of all the adjecent intersections on a street from StreetDatabase. Then we sort it and delete any duplicates before
    //returning
    sort(adjacentIntersections.begin(), adjacentIntersections.end());
    adjacentIntersections.erase(unique(adjacentIntersections.begin(), adjacentIntersections.end()), adjacentIntersections.end());
    return adjacentIntersections;
}


//find street street segments
vector<unsigned> find_street_street_segments(unsigned street_id) {

    return streetDataBase[street_id]->getSegmentVector();
}

//find intersection id from street names
vector<unsigned> find_intersection_ids_from_street_names(string street_name1, string street_name2) {
    //Gets all the intersections on all streets with the given names
    vector<unsigned int> empty;
    vector<unsigned int>intersectionStreetOne = streetNameToInterID[street_name1];
     vector<unsigned int>intersectionStreetTwo=streetNameToInterID[street_name2];
    if(empty==intersectionStreetOne||empty==intersectionStreetTwo){
        return empty;
    }
     vector<unsigned int> intersectionsFound;
//This finds the intersections which appear in both vector
     set_intersection(intersectionStreetOne.begin(), intersectionStreetOne.end(), intersectionStreetTwo.begin(), intersectionStreetTwo.end(),back_inserter(intersectionsFound));
     intersectionsFound.erase( unique( intersectionsFound.begin(), intersectionsFound.end() ), intersectionsFound.end() );
     
    return intersectionsFound;

}

//returns the street length
double find_street_length(unsigned street_id) {
    return streetDataBase[street_id]->getStreetLength();
}

//returns the length of the street segments
double find_street_segment_length(unsigned street_segment_id) {
    return segLength[street_segment_id];
}

//returns the travel time for street segments
double find_street_segment_travel_time(unsigned street_segment_id) {
    return segTravelTime[street_segment_id];
}

//returns the distance between two coordinates given by lat and lon
double find_distance_between_two_points(LatLon point1, LatLon point2) {
    return calcDistBetTwo(point1, point2);
}

//Returns the nearest point of interest to the given position
unsigned find_closest_point_of_interest(LatLon my_position)
{
    std::vector<value> NearbyPOI;
//This looks at the 700 nearest points, if there is less than 700 (like in St. Helena)
//It shrinks the vector    
    int size = 50;
    point currentPosition = point(my_position.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD, my_position.lat() * DEG_TO_RAD);
    generalRtree["poi"].query(bgi::nearest(currentPosition, size), std::back_inserter(NearbyPOI));
    int sizeOfVec = NearbyPOI.size();
    //Used to calculate the relative distance between the points and then return the closest
    //point
    double shortestDistance = calcDistBetTwoRelative(NearbyPOI[0].first, currentPosition), distance;
    unsigned int POIID = NearbyPOI[0].second;
    for (int i = 1; i < sizeOfVec; i++) {
        distance = calcDistBetTwoRelative(currentPosition, NearbyPOI[i].first);
        if (shortestDistance > distance) {

            shortestDistance = distance;
            POIID = NearbyPOI[i].second;
        }

    }
    return (POIID);
}

//Returns the the nearest intersection to the given position
unsigned find_closest_intersection(LatLon my_position) {
//This looks at the 1000 nearest points, if there is less than 700 (like in St. Helena)
//It shrinks the vector
    std::vector<value> NearbyIntersection;
    int size = 50;
    int sizeOfVec; 
    point currentPosition = point(my_position.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD, my_position.lat() * DEG_TO_RAD);
      generalRtree["intersection"].query(bgi::nearest(currentPosition, size), std::back_inserter(NearbyIntersection));
     //Used to calculate the relative distance between the points and then return the closest

    //point
    double shortestDistance = calcDistBetTwo(intersectionDataBase[NearbyIntersection[0].second]->getLatLon(), my_position), distance;
    sizeOfVec = NearbyIntersection.size();
    unsigned int intersectionID = NearbyIntersection[0].second;
    for (int i = 1; i < sizeOfVec; i++) {
        int id =NearbyIntersection[i].second;
        distance = calcDistBetTwo(my_position, intersectionDataBase[id]->getLatLon());

        if (shortestDistance > distance) {

            shortestDistance = distance;
         intersectionID = id;
        }
    }

    return (intersectionID);
}

//unsigned find_closest_intersection(LatLon my_position, int size) {
////This looks at the 1000 nearest points, if there is less than 700 (like in St. Helena)
////It shrinks the vector
//    std::vector<value> NearbyIntersection;
//    int sizeOfVec; 
//    point currentPosition = point(my_position.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD, my_position.lat() * DEG_TO_RAD);
//      generalRtree["intersection"].query(bgi::nearest(currentPosition, size), std::back_inserter(NearbyIntersection));
//     //Used to calculate the relative distance between the points and then return the closest
//    //point
//    double shortestDistance = calcDistBetTwoRelative(NearbyIntersection[0].first, currentPosition), distance;
//    sizeOfVec = NearbyIntersection.size();
//    unsigned int intersectionID = NearbyIntersection[0].second;
//    for (int i = 1; i < sizeOfVec; i++) {
//        distance = calcDistBetTwoRelative(currentPosition, NearbyIntersection[i].first);
//        if (shortestDistance > distance) {
//
//            shortestDistance = distance;
//            intersectionID = NearbyIntersection[i].second;
//        }
//    }
//
//    return (intersectionID);
//}

