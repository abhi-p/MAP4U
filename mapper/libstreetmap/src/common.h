/* 
 * File:   common.h
 * Author: sampanth
 *
 * Created on January 25, 2017, 11:46 AM
 */
#ifndef COMMON_H
#define COMMON_H
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#define numberOfIterationbeforeOverFlow 40
//Calling the boost library for the making of rtrees
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
typedef bg::model::point<double, 2, bg::cs::cartesian> point;
typedef std::pair<point, unsigned> value;
# include "intersectionNode.h"
# include "streetSegment.h"
# include "Poi.h"
# include "street.h"
# include "feature.h"
#include <unordered_map>
#include <vector>
 #include <readline/readline.h>
#include <tuple>
#include <queue>
 typedef struct node{
      bool visited=false;
      float relativeTime=0;
      vector<pair< float,int>> adjacent;
          vector<pair< float,int>> source;
      bool wantedPOINear=false;
} mapNode;

typedef struct pathSegment {
    float heuristic;
    float travelTime;
    int currInter;
    int segId;

    bool operator<(const pathSegment &o) const {
        return heuristic > o.heuristic;
    }
} pathSeg;

typedef struct  delvInfo{
    vector<int>dropOffAbleToDo;
   int totalDrops=0;
    int pickUpScore=0;
    int dropsLeft=0;

} deliverInfo;

//Global variables 
extern unordered_map <string, vector<unsigned>> streetTypeToID;
extern streetSegment** streetSegmentDataBase;
extern Poi** poiDataBase;
extern street** streetDataBase;
extern intersectionNode** intersectionDataBase;
extern feature** featureDataBase;
extern vector<vector<unsigned int> > InterToSeg;
extern vector<double> latList;
extern vector<double> xList;
extern vector<double> yList;
extern vector<pair<double, unsigned int>> featuresByArea;
extern double avgLatInter;
extern vector<unsigned int> featuresWithNoArea;
extern float maxSpeed;
extern unordered_map <string, vector<unsigned>>poiByString;
extern unordered_map <string, vector<unsigned>>interByString;
extern vector<const char*> autoComplete;
extern unordered_map <string, vector<unsigned>> streetNameToInterID;
extern vector<mapNode> graph;
extern string mapArgument;
extern int endInter;
extern priority_queue<pathSeg> pathQueue;
extern vector<pair<float, int>> travelTimes;
extern unordered_map<string,bgi::rtree< value, bgi::quadratic<numberOfIterationbeforeOverFlow> > > generalRtree;
extern vector<deliverInfo> deliver;


//closest poi
//closest intersection

 //Called repeatedly for a given `stem_text'. Each time it returns a potential
 //match. When there are no more matches it returns NULL.
 //
 //The `state' variable is zero the first time it is called with a given
 //`stem_text', and positive afterwards.
 

#endif /* COMMON_H */

