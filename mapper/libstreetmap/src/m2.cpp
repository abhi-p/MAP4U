    
#include "m2.h"
#include "m1.h"
#include "m3.h"
#include "common.h"
#include "graphics.h"
#include "Surface.h"
#include <pthread.h>
#include "fontcache.h"
#include <iostream>     // std::cin, std::cout
#include <fstream> 
#include <algorithm>
#include <string>
#include <boost/algorithm/string.hpp>
//We have three search bars in our project searchBar, allows us to go through the different search bars
//and store different values for each
typedef struct searchBar{
    vector<string> suggestions;
    string query="";
    bool change=false;
    bool searched=false;
    int moved=0;
    int firstDrawn=0;
    bool clicked=false;
    pair<int,int> xRange;
    int yDis;
} searchBarHandler;

//Works similar to a finite statemachine. Each click enables a state and
//the state the user is currently in determines what happens when the user clicks
//struct ui, contains all the states
typedef struct ui {
  
    bool searchBarSeen=true;
    bool directionsSeen=false;
    bool instructionsSeen=false;
    bool displayModes=false;
    bool displayPOIMenu=false;
    bool poiMenuShown=false;
    bool executeIntersections=false;
    string typeOfPOI;



    
   
} uiHandler;

bool hasClicked = false;
t_bound_box originalAreaBox;
int screenX;
int screenY;
vector<pair<string, unsigned int>> hoverOver;
string arrow = "->";
vector <unsigned int>foundIntersections;
vector <unsigned int>foundPOI;
vector <unsigned int>foundStreets;
vector <unsigned int>foundPath;
LatLon mouseLatLon;
bool searched=false;
uiHandler interface;
searchBarHandler sBar1,sBar2,mainBar;

unsigned find_closest_POI_By_Type(string type,LatLon my_position);
vector<string> getSuggestions(string stem_text);
void drawSuggestions(searchBarHandler & bar);
void drawDifferentSearchMode();
void drawFoundPath();
char* name_generator(const char* stem_text, int state);
char** command_completion(const char* stem_text, int start, int end);
void drawDirectionsWindow();
void drawSearchBarText(int cX,int cY, int rX, int bY,string text,string inactiveText);
bool processQuery(char key_pressed, int keySym, searchBarHandler & bar);
void drawUserInterface();
void drawInstructions();
void drawStreetType(string type);
void displayClosestIntersection();
void act_on_button_press(float x, float y, t_event_buttonPressed event);
void act_on_mousemove (float x, float y);
void drawIntersections();
void drawPOI();
void drawHighlighted();
void drawLowestSegments();
void drawStreetSegment(int index,string colour,int lineWidth);
void drawFeaturesWithArea();
void drawFeaturesWithNoArea();
void drawOneWayArrow(float firstX, float firstY, float secondX, float secondY);
void draw_screen();
void findIntersections(int location);
bool findPoi();
bool findStreets();
bool checkHoverOver();
void clearMap(void (*draws_screen)(void));
void loadNewMap(void (*draws_screen)(void));
void attempttodisplayfont();
void act_on_keypress(char key_pressed, int keysym);
void drawSearchBarText(); 
void drawInterInfo();
void drawPoiInfo();
void actOnSearch(void (*draws_screen) (void));
void ratioForPOIs();
void drawPrimaryHighways();
void checkClickOn();
void drawSecondaryHighways();
void drawExtraInfo();
void drawHoverOver();
void drawInstructions(int origin, int destination);
void helpForMap();
double currentArea;
double originalArea;
int startInter=-1;
int endInter=-1;
float interX=0;
float interY=0;
bool readyForInterPoint=false;
bool readyForPOIPoint=false;
double latforInter;
double lonforInter;

bool zoomedin=false;
int testcount=0;
int tempOne=0;
int tempTwo=0;

//takes in the user's input in the search bar, and creates
//a vector of suggestions that auto complete the user input
vector<string> getSuggestions(string stem_text){

    vector<string> suggestions;
   
    auto arrayThing=command_completion(stem_text.c_str(), 1, 100);
    
    int count=0;
    
    if(arrayThing!=NULL){
    while(arrayThing[count]!=NULL&&count<11){
    
        auto temp= arrayThing[count];
    count++;
    suggestions.push_back(temp);
    }
    }
    
    return suggestions;
}

//This function takes in the users input from any of the three search bars and talls the
//the program what to do depending on what keys are pressed
bool processQuery(char key_pressed, int keySym, searchBarHandler & bar){

    //If the user enters a backspace, get rid of the last letter/character the user has inputted.
    //the shirnk searchquery
  if (keySym == 65288&&bar.query!= "") {
            bar.query.erase(bar.query.length() - 1, 1);
            bar.query.shrink_to_fit();
            bar.firstDrawn=0;
            bar.moved=0;
            return true;
        }
         
  //If the user types any valid character push it back into the user's search query
  //then update it so a new set of suggections can be obtained from the
  //getsuggestions function. Reset the user. To improve performance, suggestions are only searched once
  //five characters are inputted
  else if (keySym> 31 && keySym < 127) {
        
       bar.query.push_back(key_pressed);
       bar.searched=false;
 if(bar.query.size()>5){
  
      bar.suggestions=getSuggestions(bar.query);

  }
       //If the user scrolls down on their serach results, reset it to the orginal search result
         bar.firstDrawn=0;
            bar.moved=0;
       return true;
     }
  
  //If the user enters the down button, they are able to navigate through the search results given
  else  if (keySym==65364){
       if(bar.moved+1<bar.suggestions.size()){
           bar.moved++;
       }
       //Shifts the texts up so the current slection is highlighted
       if(bar.firstDrawn+3<bar.suggestions.size()){
         bar.firstDrawn++;
     }      
           return true;
    }
  //Similar to above but for when the user presses the up button. It navigates
  //to the previous suggestion
  else if (keySym==65362&&bar.moved>0){
         bar.moved--;
           if(bar.firstDrawn>0){
         bar.firstDrawn--;
        } 
         return true;
        }
  
  return false;
}

//Is called when the user types in the main searchbar and presses enter. It searches for
//the nearest street, intersection or POI depending on their input
void actOnSearch(void (*draws_screen) (void)) {
  unsigned  int location = mainBar.query.find("&");
    if (location < mainBar.query.length()) {
        findIntersections(location);
    } else {
        if (!findStreets()&&!findPoi()) {
            foundIntersections.clear();
            foundPOI.clear();
            update_message("The key word \'" + mainBar.query + "\' was not found in the database");
        } else {
            update_message(mainBar.query + " was found");
        }
    }
  
    draws_screen();
}

//Called in the function above, It searches if the term serached is a POI, If that is the case it sends
//the POI ids to Found POI so they will be highlighted
bool findPoi() {
    string search = mainBar.query;
    foundIntersections.clear();

    boost::trim_right(search);
    boost::trim_left(search);

    if (poiByString[search].size() != 0) {
        foundPOI = poiByString[search];
        return true;
    }

    return false;
}

//function that checks if what is entered in the actonserach is a street
bool findStreets() {
    foundIntersections.clear();
    foundPOI.clear();
    string search = mainBar.query;
    boost::trim_right(search);
    boost::trim_left(search);

    if (find_street_ids_from_name(search).size() != 0)
 {

        sort(foundStreets.begin(), foundStreets.end());
        foundStreets.erase(unique(foundStreets.begin(), foundStreets.end()), foundStreets.end());
        foundIntersections = streetNameToInterID[search];
        set_visible_world(xList[0], yList[0], xList[getNumberOfIntersections() - 1], yList[getNumberOfIntersections() - 1]);
        return true;

    }
    return false;
}

//Called in the actOnSearch function above. If the search query is not a street or a POI, it will search to see if it is an intersection
//It takes the user input checks if thee is an &, and then divides it into two so we get the two streets
//This si used to confirm if what is enteretered is really an  intersection
void findIntersections(int location) {
    foundPOI.clear();
    string streetName1 = mainBar.query.substr(0, location - 0);
    string streetName2 = mainBar.query.substr(location + 1, mainBar.query.length() - 1);

    boost::trim_right(streetName1);
    boost::trim_left(streetName1);
    boost::trim_right(streetName2);
    boost::trim_left(streetName2);

    vector<unsigned int > intersections = find_intersection_ids_from_street_names(streetName1, streetName2);
    foundIntersections = find_intersection_ids_from_street_names(streetName1, streetName2);
    if (foundIntersections.size() == 0) {
        update_message("Intersection(s) between \"" + streetName1 + "\" and  \"" + streetName2 + "\" do not exist on this map");
    } 
    else {
        int size = foundIntersections.size();
        update_message(size + " intersection(s) found between \"" + streetName1 + "\" and \"" + streetName2);

        cout << ("Intersection(s) found between \"" + streetName1 + "\" and \"" + streetName2) + "\" are :" << endl;
        for (int i = 0; i < size; i++) {
            cout << intersectionDataBase[foundIntersections[i]]->getName() << endl;
        }
        set_visible_world(xList[0], yList[0], xList[getNumberOfIntersections() - 1], yList[getNumberOfIntersections() - 1]);
    }
}



//This takes the position of the users cursor. It converts the position into LATLON and screen coordinates
//These are global values and will be used in other parts of the program (mainly draw and check hover over)
  void act_on_mousemove (float x, float y){
      
      mouseLatLon = LatLon(y / (earth_RADIUS_IN_METERS * DEG_TO_RAD), x / (EARTH_RADIUS_IN_METERS * DEG_TO_RAD * cos(avgLatInter * DEG_TO_RAD)));
       screenX=xworld_to_scrn(x);
      screenY=yworld_to_scrn(y);
     // update_message(" Screen X is " +to_string(screenX)+" Screen Y is "+ to_string(screenY));
      draw_screen();
     
  }

//Checks if we have to do anything when the user hovers over something. This is called by drawHoverOver(below)
bool checkHoverOver(){
    
  int indexInter = find_closest_intersection(mouseLatLon);
  double distanceInter = calcDistBetTwo(intersectionDataBase[indexInter]->getLatLon(), mouseLatLon);
  int indexPOI = find_closest_point_of_interest(mouseLatLon);
  double distancePOI = calcDistBetTwo(poiDataBase[indexPOI]->getLatLon(), mouseLatLon);

    if (distanceInter <= 1) {
        pair<string, int> p ("intersection",indexInter);
        hoverOver.push_back(p);
        return true;
    } 
    if (distancePOI <= 1) {

     pair<string, int> p  ("Poi",indexPOI);
        hoverOver.push_back(p);
        return true;
    }
 
      
   if (interface.searchBarSeen)
  {
      if((screenX>=308&&screenX<=380)&&(screenY<=90&&screenY>=20))
      {
          return true;
  }
   }
  
    return false;
}

//Determines what is drawn when the user hovers over something
void drawHoverOver(){
if( checkHoverOver()){
    
    int size= hoverOver.size();
    
    if(size>0){
      
        string type= hoverOver[size-1].first;
        
        if(type=="intersection"){
            drawInterInfo();
          
        }
        
        else if(type=="Poi"){
        drawPoiInfo();
        
        }
    }


}
}

//draws the Intersection information when hovered over
void drawInterInfo(){
 set_coordinate_system(GL_SCREEN);
   unsigned int minX = 1000, maxX = 1400, minY = 10, maxY = 80;
    setcolor_by_name("white");
    int index= hoverOver[hoverOver.size()-1].second;
    fillrect(minX, minY, maxX, maxY);
    setcolor_by_name("black");
  
    string intersectionName= intersectionDataBase[index]->getName();
       drawtext(minX/2+maxX/2, maxY / 2 + minY / 2, intersectionName, maxX, maxY);

    set_coordinate_system(GL_WORLD);
}

//draws the POI information when hovered over
void drawPoiInfo(){
    set_coordinate_system(GL_SCREEN);
    unsigned int minX = 1000, maxX = 1400, minY = 10, maxY = 80;
    setcolor_by_name("white");
    int index= hoverOver[hoverOver.size()-1].second;
    fillrect(minX, minY, maxX, maxY);
    setcolor_by_name("black");
    string intersectionName= poiDataBase[index]->getName();
    drawtext(minX/2+maxX/2, maxY / 2 + minY / 2, intersectionName, maxX, maxY);
    set_coordinate_system(GL_WORLD);
}



//FSM like sturcture that allows navigation throught eh diffferent states depending on what keys the user presses
void act_on_keypress(char key_pressed, int keysym) {
if(keysym==65361)
{
    mainBar.query="";
    sBar1.query="";
    sBar2.query="";
    }
else if(keysym==65363)
{
    sBar2.query="";
     interface.instructionsSeen = false;
}
cout<<keysym<<endl;
    //If the user presses enter while in they are typing on the main serach bar, it takes the input and seraches
    //in actonsearch. 
   if(interface.searchBarSeen&&!processQuery( key_pressed,  keysym, mainBar)){
        if(keysym==65293){
          mainBar.searched=true;

            if (mainBar.suggestions.size()>0){

            actOnSearch(draw_screen);
    mainBar.query=mainBar.suggestions[mainBar.moved];
    mainBar.firstDrawn=0;
     mainBar.moved=0;
        }
   
        }}
    
    //If serachbar one (in the fdirection window is active and the user is not inputing a value)
   else if(sBar1.change&&!processQuery( key_pressed,  keysym, sBar1)){
       
      // If the user presses tab switch to the second search bar
       if(keysym==65289)
       {
         
            sBar1.change=false;
            sBar2.change=true;
           
       }
       //If the user enters the enter button the current suggestion selected is put in to
       //serach bar 1
         if(keysym==65293){
                sBar1.searched=true;

             if (sBar1.suggestions.size()>0)
             {
                 
    sBar1.query=sBar1.suggestions[sBar1.moved];
     sBar1.firstDrawn=0;
       sBar1.moved=0;
             }
         }
      
       
    }
   //Same as above but for search bar 2
    else if(sBar2.change&&!processQuery( key_pressed,  keysym, sBar2)){
   if(keysym==65293){
             sBar2.searched=true;
                          if (sBar2.suggestions.size()>0){

             sBar2.query=sBar2.suggestions[sBar2.moved];
            sBar2.firstDrawn=0;
            sBar2.moved=0;
                          }
   }
   
    }
   

    draw_screen();
}


//clears the map when exiting
void clearMap(void (*draws_screen)(void)) {
    foundIntersections.clear();
    foundPOI.clear();
    hoverOver.clear();
    draw_screen();
}

//Takes the user's input and loads the new map as per the user's input.
void loadNewMap(void (*draws_screen)(void)) {

    string previousMap = mapArgument;
    mapArgument = mainBar.query;
    boost::trim_right(mapArgument);
    boost::trim_left(mapArgument);

    close_map();
    string map_path = "/cad2/ece297s/public/maps/" + mapArgument + ".streets.bin";
//If erong input give error
    if (!load_map(map_path) && load_map("/cad2/ece297s/public/maps/" + previousMap + ".streets.bin")) {
        update_message("Our database does not have map info for \"" + mainBar.query + "\".");
        mapArgument = previousMap;
    }
    else {
        //Otherwise clear the current map and load the new map
        foundIntersections.clear();
        foundPOI.clear();
        hoverOver.clear();
        clearscreen();
        update_message("Successfully loaded map for \"" + mapArgument + "\" .");
        set_visible_world(xList[0], yList[0], xList[getNumberOfIntersections() - 1], yList[getNumberOfIntersections() - 1]);
        originalAreaBox = get_visible_world();
            originalArea = originalAreaBox.area();

    }
    draw_screen();
}

//Part of the FSM mentioned above. Is active when the user clicks the directions buttona nd is
//used so the user can fin the directions between two points

void drawDirectionsWindow() {
        settextrotation(0);
        Surface directionWindow = load_png_from_file("libstreetmap/resources/directionwindowtest.png");
        Surface directionWindowsq1 = load_png_from_file("libstreetmap/resources/directionwindowsq1.png");
        Surface directionWindowsq2 = load_png_from_file("libstreetmap/resources/directionwindowsq2.png");

        if(interface.directionsSeen)
            
  draw_surface(directionWindow, 10, 10);
        
                if(sBar1.change){
  draw_surface(directionWindowsq1, 10, 10);
                }

                if (sBar2.change){
  draw_surface(directionWindowsq2, 10, 10);
                }
    drawSearchBarText(100,84, 100, 10,sBar1.query,"Enter Intersection Name...");
   // drawSearchBarText(100,84, 100, 10,sBar1.query);
       drawSearchBarText(100,120, 100, 10,sBar2.query,"Enter Intersection Name/POI...");
 //   drawSearchBarText(100,120, 100, 10,sBar2.query);
}

void drawSearchBarText(int cX,int cY, int rX, int bY,string text,string inactiveText) {
    
    int fontSize=10;
    setfontsize(fontSize);
       
    if(interface.directionsSeen){
        setcolor_by_name("white");
    }

    if(text.size()<=0) {
       //  setcolor_by_name("black");
       
          drawtext(cX + (inactiveText.size()) * fontSize / 3-20, cY, inactiveText, 272, 40);
    }

    else if (text.size() * fontSize / 3 <= 110){
       //   setcolor_by_name("black");
        drawtext(cX + (text.size()) * fontSize / 3-20, cY, text, 272, 40);
    }

    else {
        string partialQuery = text.substr(text.size() - 30, text.size());
            drawtext(cX + (partialQuery.size()) * fontSize / 3, cY, partialQuery, 272,  40);
    }
}


void drawSuggestions(searchBarHandler & bar){
    //cout<<bar.searched<<" , "<<bar.query.size()<<endl;
    if(!bar.searched&&bar.query.size()>5){
      
        if (bar.suggestions.size()>0){
        for(int i=0;i<3&&i<bar.suggestions.size();i++){
           setcolor_by_name("white");
           fillrect(bar.xRange.first, bar.yDis+50*i, bar.xRange.second, bar.yDis+50+50*i);
           setcolor_by_name("black");
           drawtext((bar.xRange.first+bar.xRange.second)/2,(bar.yDis+50*i+bar.yDis+50+50*i)/2,  bar.suggestions[bar.firstDrawn+i], 500,80);
        }
  
       int select=(bar.moved-bar.firstDrawn);
       setcolor_by_name("yellow");
       fillrect(bar.xRange.first, bar.yDis+50*select, bar.xRange.second, bar.yDis+50+50*select);
       setcolor_by_name("black");
      drawtext((bar.xRange.first+bar.xRange.second)/2,(bar.yDis+50*select+bar.yDis+50+50*select)/2,  bar.suggestions[bar.moved], 500,80);
        }
        else {
        
        // fuzzy serach stuff
        
        }

    }
}

void drawInstructions(int origin, int destination){
    

    if ((zoomedin == false)&&(testcount <= 3)) {
        auto originXY = intersectionDataBase[origin]->getXY();
        auto destinationXY = intersectionDataBase[destination]->getXY();
        int xMin;
        int xMax;
        int yMin;
        int yMax;
        if (originXY.first <= destinationXY.first) {
            xMin = originXY.first - 20;
            xMax = destinationXY.first + 20;
        } else {
            xMin = destinationXY.first - 20;
            xMax = originXY.first + 20;
        }
        if (originXY.second <= destinationXY.second) {
            yMin = originXY.second - 20;
            yMax = destinationXY.second + 20;
        } else {
            yMin = destinationXY.second - 20;
            yMax = originXY.second + 20;
        }
    
        set_visible_world(xMin, yMin, xMax, yMax);
        zoomedin = true;
    }
    testcount++;
    set_coordinate_system(GL_SCREEN);
    
        setfontsize(9);
    settextrotation(0);
    
    int numStreetSegments=foundPath.size();
    int numDirections=1; 
    int metresTravelled=0;
    int timeTaken=0;  
    string directions;
    string numberSteps;
    string turnDirections;         
    int firstPoint=origin;
    float directionZ;
    int secondPoint=0;
    int thirdPoint=0;

       
    
   unsigned int minX = 10, maxX = 410 , minY = 152, maxY = 200;
                  
    for (int i=0; i<numStreetSegments; i++){
        
        settextrotation(0);

        string streetName= streetSegmentDataBase[foundPath[i]]->getStreetName();

 
        

            if (i < numStreetSegments - 1) {
                secondPoint = streetSegmentDataBase[foundPath[i]]->getOtherIntersectionID(firstPoint);
                thirdPoint = streetSegmentDataBase[foundPath[i + 1]]->getOtherIntersectionID(secondPoint);
                pair<double, double> commonIntersectPoint = intersectionDataBase[secondPoint]->getXY();
                pair<double, double> startPoint = intersectionDataBase[firstPoint]->getXY();
                pair<double, double> endPoint = intersectionDataBase[thirdPoint]->getXY();
                //                
                int vector1x = (commonIntersectPoint.first - startPoint.first);
                int vector1y = (commonIntersectPoint.second - startPoint.second);

                int vector2x = (endPoint.first - commonIntersectPoint.first);
                int vector2y = (endPoint.second - commonIntersectPoint.second);


                directionZ = vector1x * vector2y - vector2x * vector1y;


                if (directionZ < 0) {
                    turnDirections = "Turn right on " + streetSegmentDataBase[foundPath[i + 1]]->getStreetName();

                }

                if (directionZ > 0) {
                    turnDirections = "Turn left on " + streetSegmentDataBase[foundPath[i + 1]]->getStreetName();

                }

                if (streetSegmentDataBase[foundPath[i]]->getStreetId() != streetSegmentDataBase[foundPath[i + 1]]->getStreetId()) {
                    
        
            //must get travel time and metres of the segment before it changes
            metresTravelled+=streetSegmentDataBase[foundPath[i]]->getSegLength();
            timeTaken+=streetSegmentDataBase[foundPath[i]]->getSegTravelTime();
            
            //prints number of directions
            numberSteps=to_string(numDirections);
             
            //prints directions including metres travelled and time taken
            if (metresTravelled>1000)
                                            directions="Travel along " + streetName +" for " + to_string(metresTravelled/1000)+" km and "+to_string(metresTravelled%1000)+" m";

            else 
                            directions="Travel along " + streetName +" for " + to_string(metresTravelled)+" m";


            
            string maxChars= "blahblahblahblahblahblahblahblahblahblahblahblahblahbl";
            
            setcolor_by_name("white");
            fillrect(minX, minY, maxX, maxY);
            setcolor_by_name("black");
            
             //if text is greater than the width of the rectangle
            if (directions.length() > maxChars.length()){

            
                int difference=directions.length()-maxChars.length();
                string newDirections=directions.substr(0,maxChars.length());
                
                        Surface left = load_png_from_file("libstreetmap/resources/left.png");
                        Surface right = load_png_from_file("libstreetmap/resources/right.png");
                        Surface straight = load_png_from_file("libstreetmap/resources/straight.png");
                        if (directionZ>0)
                        {
                                           draw_surface(left, minX+20, (minY+maxY)/2);
 
                        }
                        else if (directionZ<0)
                        {
                                           draw_surface(right, minX+20, (minY+maxY)/2);
 
                        }
                        else if (turnDirections.size()==0)
                        {
                                           draw_surface(straight, minX+20, (minY+maxY)/2);
 
                        }
               drawtext(minX+10, (minY+maxY)/2, numberSteps, maxX, maxY);
                drawtext(((minX+maxX)/2)-3, (minY+maxY)/2, newDirections, maxX, maxY);
                
                minY += 50;
                maxY += 50;
                setcolor_by_name("white");
                fillrect(minX, minY, maxX, maxY);
                setcolor_by_name("black");
                
            
                drawtext(((minX+maxX)/2)-3, (minY+maxY)/2, directions.substr(maxChars.length(),difference), maxX, maxY);
                drawtext(((minX + maxX) / 2) - 3, (maxY-10), turnDirections, maxX, maxY);
                
                
                setcolor_by_name("red");

                if (timeTaken > 60) {
                    timeTaken = round(timeTaken / 60);

                    drawtext(maxX - 15, ((minY-50) + maxY) / 2, (to_string(timeTaken) + " min"), maxX, maxY);
                }
                else {
                    drawtext(maxX - 15, ((minY-50) + maxY) / 2, (to_string(timeTaken) + " s"), maxX, maxY);
                }


                minY += 50;
                maxY += 50;
                numDirections++;
                timeTaken = 0;
                metresTravelled = 0;
              
            }
            
            else{
                  Surface left = load_png_from_file("libstreetmap/resources/left.png");
                        Surface right = load_png_from_file("libstreetmap/resources/right.png");
                        Surface straight = load_png_from_file("libstreetmap/resources/straight.png");
                        if (directionZ>0)
                        {
                                           draw_surface(left, minX+20, (minY+maxY)/2);
 
                        }
                        else if (directionZ<0)
                        {
                                           draw_surface(right, minX+20, (minY+maxY)/2);
 
                        }
                         else if (turnDirections.size()==0)
                        {
                                           draw_surface(straight, minX+20, (minY+maxY)/2);
 
                        }
                
              drawtext(minX + 10, (minY + maxY) / 2, numberSteps, maxX, maxY);
                drawtext(((minX + maxX) / 2) - 3, (minY + maxY) / 2, directions, maxX, maxY);
                drawtext(((minX + maxX) / 2) - 3, (maxY-10), turnDirections, maxX, maxY);
                
                
                setcolor_by_name("red");
                if (timeTaken > 60) {
                    timeTaken = round(timeTaken / 60);
                    drawtext(maxX - 15, (minY + maxY) / 2, (to_string(timeTaken) + " min"), maxX, maxY);
                }
                else {
                    drawtext(maxX - 15, (minY + maxY) / 2, (to_string(timeTaken) + " s"), maxX, maxY);
                }

                minY += 50;
                maxY += 50;
                numDirections++;
                timeTaken = 0;
                metresTravelled = 0;

            }
   

    
        }
        
         else{
            metresTravelled+=streetSegmentDataBase[foundPath[i]]->getSegLength();
            timeTaken+=streetSegmentDataBase[foundPath[i]]->getSegTravelTime();
        }
    
    
    }
        
    else{
        
        metresTravelled+=streetSegmentDataBase[foundPath[i]]->getSegLength();
        timeTaken+=streetSegmentDataBase[foundPath[i]]->getSegTravelTime();
        setcolor_by_name("white");
            fillrect(minX, minY, maxX, maxY);
            setcolor_by_name("black");
        drawtext(minX + 10, (minY + maxY) / 2, to_string(numDirections), maxX, maxY);
        directions="Travel along " + streetSegmentDataBase[foundPath[i]]->getStreetName() +" for " + to_string(metresTravelled)+" m";
        setcolor_by_name("red");
                if (timeTaken > 60) {
                    timeTaken = round(timeTaken / 60);
                    drawtext(maxX - 15, (minY + maxY) / 2, (to_string(timeTaken) + " min"), maxX, maxY);
                }
                else {
                    drawtext(maxX - 15, (minY + maxY) / 2, (to_string(timeTaken) + " s"), maxX, maxY);
                }

        setcolor_by_name("black");
        drawtext(((minX + maxX) / 2) - 3, (minY + maxY) / 2, directions, maxX, maxY);
        minY+=50;
        maxY+=50;
        setcolor_by_name("white");
            fillrect(minX, minY, maxX, maxY);
            setcolor_by_name("black");
        directions="Arrived at destination";
        drawtext(((minX + maxX) / 2) - 3, (minY + maxY) / 2, directions, maxX, maxY);
        }


                
        firstPoint=secondPoint;   
            
           }
set_coordinate_system(GL_WORLD);
    }



void drawFoundPath()
{
        setlinewidth(4);
    int size= foundPath.size();
    string colour= "red";
    for(int i=0;i<size;i++){
    
    drawStreetSegment( foundPath[i], colour,5);
    
    }

}

void checkClickOn() {

    int index = find_closest_intersection(mouseLatLon);
    double distance = calcDistBetTwo(intersectionDataBase[index]->getLatLon(), mouseLatLon);

    int indexPOI = find_closest_point_of_interest(mouseLatLon);
    double distancePOI = calcDistBetTwo(poiDataBase[indexPOI]->getLatLon(), mouseLatLon);

    if (distance <= 1) {
       vector<unsigned int> ::iterator it;
        it = find(foundIntersections.begin(), foundIntersections.end(), index);
        if (it != foundIntersections.end()) {
            foundIntersections.erase(it);
            foundIntersections.shrink_to_fit();
        } else {
            
            if (sBar1.change)
            {
              startInter=index;
             sBar1.query=intersectionDataBase[index]->getName();
             sBar1.searched=true;  
             sBar1.clicked=true;
             
           
            }
            else if (sBar2.change)
            {
               endInter=index;
                sBar2.query=intersectionDataBase[index]->getName();
                 sBar2.searched=true;   
                              sBar2.clicked=true;

            }
            
            else if (interface.searchBarSeen)
                foundIntersections.push_back(index);
        }

    } 
    else if (distancePOI <= 1) {
        

        vector<unsigned int> ::iterator it;
        it = find(foundPOI.begin(), foundPOI.end(), indexPOI);
        if (it != foundPOI.end()) {
            foundPOI.erase(it);
            foundPOI.shrink_to_fit();
        }
        else {
            
             
         
             if (sBar2.change)
            {
               endInter=find_closest_intersection(poiDataBase[indexPOI]->getLatLon());
               sBar2.query=poiDataBase[indexPOI]->getName();
               sBar2.searched=true;   
               sBar2.clicked=true;

            }
            
            else if (interface.searchBarSeen)
            foundPOI.push_back(indexPOI);
        }
    
    
    
    }


    draw_screen();

}

void displayClosestIntersection() {
 
    int index = find_closest_intersection(mouseLatLon);
    foundIntersections.push_back(index);
   update_message("Closest Intersection is " + intersectionDataBase[index]->getName());
  
}

void act_on_button_press(float x, float y, t_event_buttonPressed event) {

    LatLon p = LatLon(y / (earth_RADIUS_IN_METERS * DEG_TO_RAD), x / (EARTH_RADIUS_IN_METERS * DEG_TO_RAD * cos(avgLatInter * DEG_TO_RAD)));
    mouseLatLon = p;
    screenX=xworld_to_scrn(x);
    screenY=yworld_to_scrn(y);
    //update_message(" Screen X is " +to_string(screenX)+" Screen Y is "+ to_string(screenY));

      
    if(interface.searchBarSeen&&(screenX>=10&&screenX<=380)&&(screenY<=49&&screenY>=14)){
        if((screenX>=308&&screenX<=380)&&(screenY<=90&&screenY>=20)&&event.button == 1)
        {
                   update_message("in search bar ");
                   interface.directionsSeen=true;
                   interface.searchBarSeen=false;
                      interface.displayModes=false;
        }

//  
             if((screenX>=11&&screenX<=51)&&(screenY<=54&&screenY>=14)&&event.button == 1){

        update_message("different modes available");
        interface.displayModes=true;
        interface.searchBarSeen=false;
       interface.directionsSeen=false;

     
     
 }
    }


    //if you want to return from choosing a mode
    if(interface.displayModes &&(screenX>=11&&screenX<=297)&&(screenY<=132&&screenY>=23)&&event.button == 1){
        
        
          if ((screenX >= 11 && screenX <= 297)&&(screenY <= 88 && screenY >= 54)) {
              // Display poi menu
        interface.displayPOIMenu=true;
        interface.displayModes=false;
            interface.executeIntersections = false;
         }
        
             
           if((screenX>=11&&screenX<=297)&&(screenY<=132&&screenY>=88)){
     // display intersections menu
            interface.executeIntersections = true;
        
             }
        
          
          if((screenX>=258&&screenX<=270)&&(screenY<=32&&screenY>=23)){
              // exit button
        interface.displayModes=false;
        interface.searchBarSeen=true;
            interface.executeIntersections = false;
          }
        
    }


   

    if(interface.displayPOIMenu && (screenX >= 12 && screenX <= 293)&&(screenY <= 152 && screenY >= 12) && event.button == 1){

        //not true yet, since user hasnt entered coordinate
    
        
         if((screenX >= 35 && screenX <= 45)&&(screenY <= 36&& screenY >= 27) ){
             // exit button
        interface.displayPOIMenu=false;
        foundPath.clear();
        interface.displayModes=true;
    }    
         
         if((screenX >= 11 && screenX <= 300)&&(screenY <= 80 && screenY >= 58)){
             
             interface.typeOfPOI="hospital";
   
         }
         if((screenX >= 11 && screenX <= 300)&&(screenY <= 113 && screenY >= 81)){
         
             interface.typeOfPOI="restaurant";
      
         }
         if((screenX >= 11 && screenX <= 300)&&(screenY <= 142 && screenY >= 114)){
            interface.typeOfPOI="general";
         
         }
         
         
    }
    


  if(interface.directionsSeen&&(screenX>=10&&screenX<=373)&&(screenY<=149&&screenY>=10)){
                  
                     if((screenX>=338&&screenX<=352)&&(screenY>=10&&screenY<=46)&&event.button == 1)
                    {
                         // exit button
            interface.directionsSeen = false;
            interface.instructionsSeen = false;
            foundPath.clear();
            startInter = -1;
            endInter = -1;
            sBar1.searched = false;
            sBar2.searched = false;
            testcount = 0;
            zoomedin = false;
            //sBar1.query
            sBar1.query = "";
            sBar2.query="";
            startInter=-1;
            endInter=-1;
                   interface.searchBarSeen=true;
                     }
                      if((screenX>=335&&screenX<=354)&&(screenY>=86&&screenY<=108)&&event.button == 1)
                    {
                          // switch button
                          string temp= sBar1.query;
                          sBar1.query=sBar2.query;
                          sBar2.query=temp;
                          int tempInter =startInter;
                          startInter=endInter;
                          endInter=tempInter;
                      }

                     
               if((screenX>=329&&screenX<=365)&&(screenY>=132&&screenY<=144)&&event.button == 1)
                    {
                         //find button
                  
                   
                   if ((interByString[sBar1.query].size())>0&&(interByString[sBar2.query].size())>0)
                   {
                       if(!sBar1.clicked){
                              startInter=interByString[sBar1.query][0];
                                      }
                           if(!sBar2.clicked) {
                           endInter= interByString[sBar2.query][0];
                           }
                             
                       foundPath= find_path_between_intersections(startInter,endInter, 20);

                               interface.instructionsSeen=true;
                           }
                           else if((interByString[sBar1.query].size())>0&&(poiByString[sBar2.query].size())>0)
                           {
                                                        if(!sBar1.clicked){
                           startInter=interByString[sBar1.query][0];
                                                        }
                           foundPath= find_path_to_point_of_interest(startInter,sBar2.query,20);

                           interface.instructionsSeen=true;
                           } 
                           
                           else
                           {
                           update_message(" invalid intersections/POI");
                           }

                
               
               
               
               }
                     
                     
                    if((screenX>=75&&screenX<=312)&&event.button == 1){
                        
                        if(screenY>=75&&screenY<=98){
                             // type in sBar1
                            sBar1.change=true;
                            sBar2.change=false;
                        }
                        
                        if(screenY>=116&&screenY<=134){
                           // type in sBar2
                            sBar1.change=false;
                            sBar2.change=true;
                        }
                        
                        }

    }
     if(event.button == 3)
         checkClickOn();

  draw_screen();


}

void drawIntersections() {

    Surface mapPin = load_png_from_file("libstreetmap/resources/mappin.png");

    int numOfInter = getNumberOfIntersections();
    setcolor(LIGHTGREY);
    for (int i = 0; i < numOfInter; i++) {
        pair<double, double> p1 = intersectionDataBase[i]->getXY();
        fillellipticarc(p1.first, p1.second, 1, 1, 0, 360);
    }
}

void drawPOI() {


    int numOfPoi = getNumberOfPointsOfInterest();
    Surface medical = load_png_from_file("libstreetmap/resources/red.png");
    Surface fuel = load_png_from_file("libstreetmap/resources/fuel.png");
    Surface resturant = load_png_from_file("libstreetmap/resources/resturant.png");
    Surface bank = load_png_from_file("libstreetmap/resources/bank.png");
    Surface education = load_png_from_file("libstreetmap/resources/education.png");
    Surface police = load_png_from_file("libstreetmap/resources/police.png");
    Surface bus = load_png_from_file("libstreetmap/resources/bus.png");
    Surface library = load_png_from_file("libstreetmap/resources/library.png");
    Surface firestation = load_png_from_file("libstreetmap/resources/firestation.png");
    Surface parking = load_png_from_file("libstreetmap/resources/parking.png");
    Surface pub = load_png_from_file("libstreetmap/resources/pub.png");
    Surface pharmacy = load_png_from_file("libstreetmap/resources/pharmacy.png");
    Surface praying = load_png_from_file("libstreetmap/resources/praying.png");
    Surface mapPin = load_png_from_file("libstreetmap/resources/mappin.png");

    setcolor(58, 45, 28, 255);

    settextrotation(0);
    
    for (int i = 0; i < numOfPoi; i++) {
        string type = poiDataBase[i]->getType();

        pair<double, double> p1 = poiDataBase[i]->getXY();
    //    cout<<currentArea / originalArea<<endl;

        /*=============LAYER TWO===============*/
        if ((currentArea / originalArea) <= 0.01 &&(currentArea / originalArea) > 0.0055) {


            if (type == "hospital" || type == "doctors" || type == "dentist" || type == "clinic") {
                draw_surface(medical, p1.first, p1.second);
            }
            else if (type == "police")
                draw_surface(police, p1.first, p1.second);
            else if (type == "fire_station")
                draw_surface(firestation, p1.first, p1.second);
            

        }
        /*=============LAYER THREE===============*/

    

        else  if ((currentArea / originalArea) <= 0.0055 && (currentArea / originalArea) > 0.000925) {
   if (type == "hospital" || type == "doctors" || type == "dentist" || type == "clinic") {
                draw_surface(medical, p1.first, p1.second);
            }
            else if (type == "police")
                draw_surface(police, p1.first, p1.second);
            else if (type == "fire_station")
                draw_surface(firestation, p1.first, p1.second);


            else if (type == "bank") {
                draw_surface(bank, p1.first, p1.second);
            } else if (type == "college" || type == "school" || type == "kindergarten") {
                draw_surface(education, p1.first, p1.second);
            } else if (type == "library") {
                draw_surface(library, p1.first, p1.second);
            } else if (type == "pharmacy")
                draw_surface(pharmacy, p1.first, p1.second);
            else if (type == "place_of_worship")
                draw_surface(praying, p1.first, p1.second);
        

        }

        /*=============LAYER FOUR===============*/
        else if ((currentArea / originalArea) <= 0.000925) {
   if (type == "hospital" || type == "doctors" || type == "dentist" || type == "clinic") {
                draw_surface(medical, p1.first, p1.second);
            }
            else if (type == "police")
                draw_surface(police, p1.first, p1.second);
            else if (type == "fire_station")
                draw_surface(firestation, p1.first, p1.second);


            else if (type == "bank") {
                draw_surface(bank, p1.first, p1.second);
            } else if (type == "college" || type == "school" || type == "kindergarten") {
                draw_surface(education, p1.first, p1.second);
            } else if (type == "library") {
                draw_surface(library, p1.first, p1.second);
            } else if (type == "pharmacy")
                draw_surface(pharmacy, p1.first, p1.second);
            else if (type == "place_of_worship")
                draw_surface(praying, p1.first, p1.second);
            else if (type == "fuel") {
                draw_surface(fuel, p1.first, p1.second);
            } else if (type == "fast food" || type == "cafe" || type == "restaurant" || type == "food_court" || type == "ice_cream")
                draw_surface(resturant, p1.first, p1.second);
            else if (type == "parking")
                draw_surface(parking, p1.first, p1.second);
            else if (type == "bus_station")
                draw_surface(bus, p1.first, p1.second);
            else if (type == "pub" || type == "bar")
                draw_surface(pub, p1.first, p1.second);
            else {
                setcolor_by_name("purple");
                fillellipticarc(p1.first, p1.second, 2, 2, 0, 360);

            }

       }

        setcolor_by_name("black");

        string poiName = poiDataBase[i]->getName();
        setfontsize(7);

        drawtext(p1.first, p1.second, poiName, 100, 100);

    }



}

void drawStreetType(string type){
int size= streetTypeToID[type].size();
vector<unsigned int> segments= streetTypeToID[type];
        setcolor_by_name(colourDecider(type));
    
for (int i = 0; i < size; i++) {
            drawStreetSegment(segments[i],colourDecider(type),streetSegmentDataBase[segments[i]]->getFontSize());
        settextrotation(0);
        setlinewidth(1);
    }
setcolor_by_name(colourDecider(type));
}


void drawLowestSegments() {
  //  setcolor_by_name(colourDecider("tertiary"));
drawStreetType("tertiary");
drawStreetType("tertiary_link");
drawStreetType("residential");
drawStreetType("service");
drawStreetType("others");
}

void drawSecondaryHighways() {
  //  setcolor_by_name(colourDecider("secondary"));
drawStreetType("secondary");
drawStreetType("secondary_link");
}

void drawPrimaryHighways() {
  //  setcolor_by_name(colourDecider("motorway"));
    drawStreetType("motorway");
    drawStreetType("motorway_link");
    drawStreetType("trunk" );
    drawStreetType("trunk_link" );
    drawStreetType("primary"  );
    drawStreetType("primary_link" );

}

void drawFeaturesWithArea() {
    int numOfPolygons = featuresByArea.size();

    for (int i = (numOfPolygons - 1); i>-1; i--) {
        int index = featuresByArea[i].second;
        setcolor_by_name(featureDataBase[index]->getColour());
        fillpoly(featureDataBase[index]->getPointsPointer(), featureDataBase[index]->getNumOfPoints());
    }

}

void drawFeaturesWithNoArea() {
    int numOfPolygons = featuresWithNoArea.size();


    for (int i = 0; i < numOfPolygons; i++) {
        int index = featuresWithNoArea[i];
        setcolor_by_name(featureDataBase[index]->getColour());
        int numOfPoints = featureDataBase[index]->getNumOfPoints();
        t_point point1 = featureDataBase[index]->getPoint(0), point2;
        for (int j = 1; j < numOfPoints; j++) {
            point2 = featureDataBase[index]->getPoint(j);
            drawline(point1, point2);
            point1 = point2;
        }

    }

}

void drawHighlighted() {
    Surface mapPin = load_png_from_file("libstreetmap/resources/mappin.png");
    int size = foundPOI.size();

    for (int i = 0; i < size; i++) {
        pair<double, double> highlightedPOI = poiDataBase[foundPOI[i]]->getXY();
        draw_surface(mapPin, highlightedPOI.first-0.75, highlightedPOI.second+0.75);

    }
    size = foundIntersections.size();

    for (int i = 0; i < size; i++) {

        pair<double, double> p1 = intersectionDataBase[foundIntersections[i]]->getXY();
        
        draw_surface(mapPin, p1.first-0.75, p1.second+0.75);

    }
   Surface startPin = load_png_from_file("libstreetmap/resources/start_pin.png");
       Surface endPin = load_png_from_file("libstreetmap/resources/endpin.png");
       if (interface.directionsSeen){
   if(startInter>=0){
            draw_surface(startPin, intersectionDataBase[startInter]->getXY().first, intersectionDataBase[startInter]->getXY().second);
    }
          if(endInter>=0){
            draw_surface(endPin, intersectionDataBase[endInter]->getXY().first, intersectionDataBase[endInter]->getXY().second);
   }
       }
}

void drawStreetSegment(int index,string colour,int lineWidth){
    setcolor_by_name(colour);
setlinewidth(lineWidth);
            int numOfCurve = streetSegmentDataBase[index]->getNumOfCurvePoints();
            int id1 = streetSegmentDataBase[index]->getFrom();
            LatLon point1 = intersectionDataBase[id1]->getLatLon();
            double firstX = earth_RADIUS_IN_METERS * point1.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD;
            double firstY = earth_RADIUS_IN_METERS * point1.lat() * DEG_TO_RAD;
            vector<LatLon> curvePoints = streetSegmentDataBase[index]->getCurvePointVector();
            double secondX, secondY;

            setlinestyle(0, 0);

            for (int j = 0; j < numOfCurve; j++) {
                secondX = earth_RADIUS_IN_METERS * curvePoints[j].lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD;
                secondY = earth_RADIUS_IN_METERS * curvePoints[j].lat() * DEG_TO_RAD;
                drawline(firstX, firstY, secondX, secondY);

                if (streetSegmentDataBase[index]->isOneWay()) {
              drawOneWayArrow( firstX, firstY, secondX, secondY);
                      setcolor_by_name(colour);

                }
               
                firstX = secondX;
                firstY = secondY;

            }

            id1 = streetSegmentDataBase[index]->getTo();
            point1 = intersectionDataBase[id1]->getLatLon();
            secondX = earth_RADIUS_IN_METERS * point1.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD;
            secondY = earth_RADIUS_IN_METERS * point1.lat() * DEG_TO_RAD;
            drawline(firstX, firstY, secondX, secondY);
            if (streetSegmentDataBase[index]->isOneWay()) {
                drawOneWayArrow( firstX, firstY, secondX, secondY);
                 setcolor_by_name(colour);

            }
             string name= streetSegmentDataBase[index]->getStreetName();
                  if(name!="<unknown>"){
                    setfontsize(9);
                    setcolor_by_name("black");
                      settextrotation((atan((secondY - firstY)/(secondX - firstX))) / deg_TO_RAD1);
                      drawtext((firstX+secondX)/2, (firstY+secondY)/2, name, 100, 100);
                    setcolor_by_name(colour);
                }
}

void drawOneWayArrow(float firstX, float firstY, float secondX, float secondY) {

    setcolor_by_name("black");
    setfontsize(6);
    settextrotation((atan2((secondY - firstY), (secondX - firstX))) / DEG_TO_RAD);
    drawtext(firstX / 2 + secondX / 2, firstY / 2 + secondY / 2, arrow, 1000, 1000);
     settextrotation(0);
  
}

void attempttodisplayfont() {

    setcolor(91, 70, 42, 255);

    string streetname;

    for (unsigned int i = 0; i < getNumberOfStreetSegments(); i++) {
        streetname = streetSegmentDataBase[i]->getStreetName();
        if (streetname != "<unknown>") {


            LatLon start = getIntersectionPosition(streetSegmentDataBase[i]->getFrom());
            LatLon finish = getIntersectionPosition(streetSegmentDataBase[i]->getTo());
            double posOneLat = earth_RADIUS_IN_METERS * deg_TO_RAD1 * start.lat();
            double posTwoLat = earth_RADIUS_IN_METERS * deg_TO_RAD1 * finish.lat();
            double posOneLon = earth_RADIUS_IN_METERS * deg_TO_RAD1 * start.lon();
            double posTwoLon = earth_RADIUS_IN_METERS * deg_TO_RAD1 * finish.lon();


            double x1 = posOneLon * cos(deg_TO_RAD1 * avgLatInter);
            double x2 = posTwoLon * cos(deg_TO_RAD1 * avgLatInter);
            double y1 = posOneLat;
            double y2 = posTwoLat;


            double xc = ((x2 - x1) / 2) + x1;
            double yc = ((y2 - y1) / 2) + y1;


            setfontsize(9);

            settextrotation((atan2((y2 - y1), (x2 - x1))) / deg_TO_RAD1);

            drawtext(xc, yc, streetname, 100, 100);

        }
    }
    settextrotation(0);
}

void ratioForPOIs() {

    /*=============LAYER ONE===============*/
    if ((currentArea / originalArea) <= 1) {
        drawPrimaryHighways();
        drawIntersections();
        drawPOI();
    }


    /*=============LAYER TWO===============*/
     if ((currentArea / originalArea) <= 0.20) {
       drawSecondaryHighways();
        }
    
    /*=============LAYER THREE===============*/
     if ((currentArea / originalArea) < 0.001) {
        drawLowestSegments();      
}
}

void draw_screen() {

    t_bound_box areaBox = get_visible_world();

    currentArea = areaBox.area();

    set_drawing_buffer(OFF_SCREEN);
    clearscreen();
    drawFeaturesWithArea();
    drawFeaturesWithNoArea();

    /*=====drawing aspects on specific zoom levels============*/
    ratioForPOIs();
    drawIntersections();
 
   drawFoundPath();
    drawHighlighted();
  // drawInstructions(5,5);
    drawUserInterface();
    drawHoverOver();


    copy_off_screen_buffer_to_screen();


}

void drawUserInterface(){
set_coordinate_system(GL_SCREEN);
   

    if (interface.searchBarSeen) {
 
    
        update_message("Welcome! You can search for any street, POI or intersection here or go to the directions or menu screens");
        Surface searchBar = load_png_from_file("libstreetmap/resources/searchbar.png");
        draw_surface(searchBar, 10, 10);
        drawSearchBarText(80, 35, 100, 10, mainBar.query,"Enter Intersection/POI/Street...");
      //  drawSearchBarText(80, 35, 100, 10, mainBar.query);

  if((screenX>=308&&screenX<=380)&&(screenY<=90&&screenY>=20))
      {
      Surface searchBarDirections = load_png_from_file("libstreetmap/resources/testing.png");
        draw_surface(searchBarDirections, 10, 10);

  }
       if((screenX>=11&&screenX<=51)&&(screenY<=54&&screenY>=14))
      {
            
      Surface searchBarDirections = load_png_from_file("libstreetmap/resources/searchbar_menu.png");
        draw_surface(searchBarDirections, 10, 10);
  
  }


        if (!mainBar.searched)
            drawSuggestions(mainBar);
        }

   
if (interface.directionsSeen) {
              update_message("Click on one of the search bars to begin input. You may type in your input or right click on the map. When you are done click find to get directions.");

        drawDirectionsWindow();

        
    

    if (sBar1.change) {
update_message("Type in or right click on the map to set your origin intersection");

        if (!sBar1.searched)
            drawSuggestions(sBar1);
    }

    if (sBar2.change) {
  update_message("Type in an intersection or POI or right click on the map to set your destination");

        if (!sBar2.searched)
            drawSuggestions(sBar2);
    }
}
if (interface.instructionsSeen) {
        drawInstructions(startInter, endInter);
        }

   

    if (interface.displayModes) {

        Surface modes = load_png_from_file("libstreetmap/resources/menu.png");
        draw_surface(modes, 10, 10);
        //drawDifferentSearchMode(); 
    }
    
     if (interface.displayPOIMenu){
        Surface poiType=load_png_from_file("libstreetmap/resources/poi-types.png");
        if(interface.typeOfPOI=="hospital")
        { Surface poimedical = load_png_from_file("libstreetmap/resources/poi-medical.png");
               
        draw_surface(poimedical, 10,10);}
        else if(interface.typeOfPOI=="restaurant")
        { Surface poiresturant = load_png_from_file("libstreetmap/resources/poi-resturants.png");
               
        draw_surface(poiresturant, 10,10);}
        else if(interface.typeOfPOI=="general")
        { Surface anypoi = load_png_from_file("libstreetmap/resources/anyPoi.png");
               
        draw_surface(anypoi, 10,10);
        }
        else
        draw_surface(poiType, 10,10);
           
            int  poiClosest=0;
            update_message("click any point on the map");

            if(interface.typeOfPOI=="general"){
                poiClosest=find_closest_point_of_interest(mouseLatLon);
            }
        
            else{
                
                poiClosest= find_closest_POI_By_Type(interface.typeOfPOI,mouseLatLon);
                
            }
                
                update_message("Point Received");
                
               
                
                
                unsigned int minX = 10, maxX = 300 , minY = 140, maxY = 210;
                setcolor_by_name("white");
                fillrect(minX, minY, maxX, maxY);
                setcolor_by_name("black");
                setfontsize(9);
                settextrotation(0);
                drawtext(((minX+maxX)/2)-3, ((minY+(160))/2)+9, "Name: "+poiDataBase[poiClosest]->getName(), maxX, maxY);
                LatLon poiPoint=poiDataBase[poiClosest]->getLatLon();
                drawtext(((minX+maxX)/2)-3, ((minY+maxY)/2)+9, "Lat Lon is: " +to_string(poiPoint.lat())+" " +to_string(poiPoint.lon()), maxX, maxY);
                update_message("");
                
            }
        

    

   if (interface.executeIntersections){
    
 
         
            update_message("click any point on the map");
       
                //point gotten
                update_message("Point Received");
               // cout << interX << " " << interY << endl;
            
                int interClosest=find_closest_intersection(mouseLatLon);
               // cout<<intersectionDataBase[interClosest]->getName()<<endl;
                
                pair<double, double> p1 = intersectionDataBase[interClosest]->getXY();
                Surface mapPin = load_png_from_file("libstreetmap/resources/mappin.png");
                draw_surface(mapPin, p1.first-0.75, p1.second+0.75);
                
                
                unsigned int minX = 10, maxX = 300 , minY = 133, maxY = 203;
                setcolor_by_name("white");
                fillrect(minX, minY, maxX, maxY);
                setcolor_by_name("black");
                setfontsize(9);
                
                drawtext(((minX+maxX)/2)-3, ((minY+160)/2)+9, "Name: "+intersectionDataBase[interClosest]->getName(), maxX, maxY);
                LatLon interPoint=intersectionDataBase[interClosest]->getLatLon();
                drawtext(((minX+maxX)/2)-3, ((minY+maxY)/2)+9, "Lat Lon is: " +to_string(interPoint.lat())+" " +to_string(interPoint.lon()), maxX, maxY);

                update_message("");

    }
    set_coordinate_system(GL_WORLD);

 }

void helpForMap(){
    
    cout<<"Help for Map4U"<<endl<<endl;
    cout<<"**********MODES**********"<<endl;
    cout<<"This map offers two modes for finding any close points of interest or intersection. These options are available by "
            "clicking the three lines next to the search bar. "<<endl<<"For finding the closest point of interest, there are three options:"
            " Medical, restaurants and any general POI. Each can be chosen by clicking on it. For both intersections and POIs,"
            "the"<<endl<<" closest one is given depending on where the mouse is."<<endl<<endl;
    cout<<"**********SEARCH BAR**********"<<endl;
    cout<<"The search bar is available for use once the map is launched. One can begin to search a place where they would like to "
            "go and predicted text will be provided as the user types. These options can be selected by hitting the enter key."
            "Once a place is entered, a pin appears at the destination."<<endl<<endl;
    cout<<"**********DIRECTIONS**********"<<endl;
    cout<<"Directions will be given by clicking the arrow next to the search bar. A direction menu then appears which prompts "
            "the user to enter two intersections or point of interests." <<endl<<"The user can either type in the places, or click"
            "areas on the map for it. Auto-complete is also provided, but the user must select the generated" <<endl<<"option and hit "
            "enter, not click." <<endl<<"Once find is clicked, directions are generated."<<endl;
            
    
}

void draw_map() {

        helpForMap();
    t_color background(239, 238, 234, 255);
    init_graphics(mapArgument, background);
    create_button("Window", "Find", actOnSearch);
    create_button("Find", "Clear", clearMap);
    create_button("Clear", "Load Map", loadNewMap);
    set_keypress_input(true);
    set_mouse_move_input(true);
    pair<int,int>x (76,316);
    sBar1.xRange=x;
    sBar2.xRange=x;
    x.first=56;
    x.second=306;
    mainBar.xRange=x;
    sBar1.yDis=94;
    sBar2.yDis=130;
    mainBar.yDis=52;
    
    
    set_visible_world(xList[0], yList[0], xList[getNumberOfIntersections() - 1], yList[getNumberOfIntersections() - 1]);
    originalAreaBox = get_visible_world();
    originalArea = originalAreaBox.area();
    event_loop(act_on_button_press, act_on_mousemove, act_on_keypress, draw_screen);

    close_graphics();
}


//unsigned levenshtein_distance(const std::string source, const std::string target);
//
//unsigned levenshtein_distance(const std::string source, const std::string target) {
// vector<vector<unsigned> > matrix;
// 
// const unsigned n=source.length();
// const unsigned m=target.length();
// 
// matrix.resize(n+1); 
//
//  for (int i = 0; i <= n; i++) {
//    matrix[i].resize(m+1);
//  }
// 
// for (unsigned i = 0; i < matrix.size(); i++) {
//        matrix[i].resize(s2.length() + 1); 
//    }
// 
// 
//
//    for (unsigned i = 0; i < source.length() + 1; i++) {
//        for (unsigned j = 0; j < target.length() +1; j++) {
//            matrix[i][j] = 0;
//        }
//    }
//
//    for (unsigned i = 1; i < matrix.size(); i++) {
//        matrix[i][0] = i;
//    }
//
//    for (unsigned j = 1; j < matrix[0].size(); j++) {
//        matrix[0][j] = j;
//    }
//
//    unsigned sub_cost = 0;
//
//    for (unsigned j = 1; j < matrix[0].size(); j++) {
//        for (unsigned i = 1; i < matrix.size(); i++) {
//            if (s1[i-1] == s2[j-1])
//                sub_cost = 0;
//            else
//                sub_cost = 1;
//
//            matrix[i][j] = std::min(std::min(matrix[i-1][j] + 1, matrix[i][j-1] +1), matrix[i-1][j-1] + sub_cost);
//        }
//    }
//    return matrix[matrix.size() -1][matrix[0].size() -1];
//}
char* name_generator(const char* stem_text, int state) {
 //Static here means a variable's value persists across function invocations
 static int count;

 if(state == 0) {

 count = -1;
 }

 int text_len = strlen(stem_text);

 //Search through intersection_names until we find a match
 while(count < (int) autoComplete.size()-1) {
 count++;
 if(strncmp(autoComplete[count], stem_text, text_len) == 0) {
//Must return a duplicate, Readline will handle
 //freeing this string itself.
 return strdup(autoComplete[count]);
}
 }

 return NULL;
 }
char** command_completion(const char* stem_text, int start, int end) {
 char ** matches = NULL;

 if(start != 0) {
 //Only generate completions if `stem_text'
 //is not the first thing in the buffer
 matches = rl_completion_matches(stem_text, name_generator );
 }

 return matches;
}
unsigned find_closest_POI_By_Type(string type,LatLon my_position) {
   
//This looks at the 1000 nearest points, if there is less than 700 (like in St. Helena)
//It shrinks the vector
    std::vector<value> nearByPoi;
    int size = 50;
    int sizeOfVec; 
    point currentPosition = point(my_position.lon() * cos(avgLatInter * DEG_TO_RAD) * DEG_TO_RAD, my_position.lat() * DEG_TO_RAD);
      generalRtree[type].query(bgi::nearest(currentPosition, size), std::back_inserter(nearByPoi));
     //Used to calculate the relative distance between the points and then return the closest
    //point
      int poiId=0;
      if(nearByPoi.size()>0){
          poiId= nearByPoi[0].second;
    double shortestDistance = calcDistBetTwo(poiDataBase[nearByPoi[0].second]->getLatLon(), my_position), distance;
   
    sizeOfVec = nearByPoi.size();
    
    for (int i = 1; i < sizeOfVec; i++) {
        distance = calcDistBetTwo (my_position, poiDataBase[nearByPoi[i].second]->getLatLon());
        if (shortestDistance > distance) {

            shortestDistance = distance;
            poiId = nearByPoi[i].second;
        }
    }
}
 foundPath=find_path_to_point_of_interest(find_closest_intersection(my_position), poiDataBase[poiId]->getName(),20);
    return (poiId);
}
