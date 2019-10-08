#pragma once //protects against multiple inclusions of this header file

#include <string>
#include "LatLon.h"

#include "Feature.h"
#include "OSMID.h"


/*********************************************************************************
 * LAYER-2 API (libstreetsdatabase) INTRODUCTION
 *
 * The libstreetsdatabase "layer 2" API provides a simplified way of interacting 
 * with the OSM map data. For your convenience, we have extracted a subset of the 
 * information in the OSM schema of nodes, ways, and relations with attributes and 
 * pre-processed it into a form that is easier to use, consisting of familiar 
 * concepts like streets, intersections, points of interest, etc. You should start 
 * working with this layer first.
 *
 * The streets database is generated by the provided "osm2bin" tool, and stored in 
 * a binary file called {cityname}.streets.bin.
 *
 * For access to additional feature types and attribute information, you can use
 * the underlying "layer 1" API which presents the OSM data model without 
 * modification. It is more flexible but less easy to understand, however there 
 * are many resources on the web including the OSM wiki and interactive online 
 * viewers to help you.
 *
 * The "layer 1" API is described in OSMDatabaseAPI.h. To match objects between 
 * layers, the this API provides OSM IDs for all objects.
 */

// load a {map}.streets.bin file. This function must be called before any 
// other function in this API can be used. Returns true if the load succeeded,
// false if it failed.
bool loadStreetsDatabaseBIN(std::string fn);

// unloads a map / frees the memory used by the API. No other api calls can
// be made until the load function is called again for some map.
void closeStreetDatabase(); 



/** The extracted objects are:
 *
 * Intersections      A point (LatLon) where a street terminates, or meets one 
                      or more other streets
 * Street segments    The portion of a street running between two intersections
 * Streets            A named, ordered collection of street segments running 
                      between two or more intersections
 * Points of Interest (POI)   Points of significance (eg. shops, tourist 
                      attractions) with a LatLon position and a name
 * Features           Marked polygonal areas which may have names (eg. parks, 
                      bodies of water)
 *
 *
 * Each of the entities in a given map file is labeled with an index running from 
 * 0..N-1 where N is the number of entities of that type in the map database that 
 * is currently loaded. These indices are not globally unique; they depend on the 
 * subset of objects in the present map, and the order in which they were loaded 
 * by osm2bin.
 *
 * The number of entities of each type can be queried using getNumberOf[...], 
 * eg. getNumberOfStreets() 
 * Additional information about the i'th entity of a given type can be accessed 
 * with the functions defined in the API below.
 *
 * A std::out_of_range exception is thrown if any of the provided indices are 
 *  invalid.
 *
 * Each entity also has an associated OSM ID that is globally unique in the OSM 
 * database, and should never change. The OSM ID of the OSM entity (Node, Way, or 
 * Relation) that produced a given feature is accessible. You can use this OSMID 
 * to access additional information through attribute tags, and to coordinate 
 * with other OSM programs that use the IDs.
 */


/** For clarity reading the API below, the index types are all typedef'ed from 
 * unsigned. Valid street indices range from 0 .. N-1 where N=getNumberOfStreets()
 */

typedef unsigned FeatureIndex;
typedef unsigned POIIndex;
typedef unsigned StreetIndex;
typedef unsigned StreetSegmentIndex;
typedef unsigned IntersectionIndex;

unsigned getNumberOfStreets();
unsigned getNumberOfStreetSegments();
unsigned getNumberOfIntersections();
unsigned getNumberOfPointsOfInterest();
unsigned getNumberOfFeatures();



/*********************************************************************************
 * Intersection
 *
 * Each intersection has at least one street segment incident on it. Each street 
 * segment ends at another intersection.
 *
 * Names are generated by concatenating the incident street names with an 
 * ampersand, eg. "Yonge" + "Bloor" = "Yonge & Bloor"
 * Where the intersection name is not unique, a numerical identifier is appended, 
 * eg. "Yonge & Bloor (1)". The order is arbitrarily assigned when the 
 * .streets.bin file is generated. Names are therefore unique.
 */

std::string    getIntersectionName(IntersectionIndex intersectionIdx);
LatLon         getIntersectionPosition(IntersectionIndex intersectionIdx);
OSMID          getIntersectionOSMNodeID(IntersectionIndex intersectionIdx);

// access the street segments incident on the intersection (get the count Nss 
// first, then iterate through i=0..Nss-1)
unsigned     getIntersectionStreetSegmentCount(IntersectionIndex intersectionIdx);
StreetSegmentIndex getIntersectionStreetSegment(IntersectionIndex intersectionIdx, unsigned i); 



/*********************************************************************************
 * Street segment
 *
 * A street segment connects two intersections. It has a speed limit, from- and 
 * to-intersections, and an associated street (which has a name).
 *
 * When navigating or drawing, the street segment may have zero or more "curve 
 * points" that specify its shape.
 * 
 * Information about the street segment is returned in the StreetSegmentInfo 
 * struct defined below.
 */

struct StreetSegmentInfo {
    OSMID wayOSMID;   // OSM ID of the source way
                      // NOTE: Multiple segments may match a single OSM way ID

    IntersectionIndex from, to;  // intersection ID this segment runs from/to
    bool oneWay;            // if true, then can only travel in from->to direction

    unsigned curvePointCount;    // number of curve points between the ends
    float speedLimit;            // in km/h

    StreetIndex	streetID;        // index of street this segment belongs to
};

StreetSegmentInfo getStreetSegmentInfo(StreetSegmentIndex streetSegmentIdx);

// fetch the latlon of the i'th curve point (number of curve points specified in 
// StreetSegmentInfo)
LatLon getStreetSegmentCurvePoint(StreetSegmentIndex streetSegmentIdx, unsigned i);



/*********************************************************************************
 * Street
 *
 * A street is made of multiple StreetSegments, which hold most of the 
 * fine-grained information (one-way status, intersections, speed limits...). 
 * The street is just a named identifier for a collection of segments.
 */

std::string getStreetName(StreetIndex streetIdx);




/*********************************************************************************
 * Points of interest
 *
 * Points of interest are derived from OSM nodes. More detailed information can be 
 * accessed from the layer-1 API using the OSM ID.
 */

std::string getPointOfInterestType(POIIndex poiIdx);
std::string getPointOfInterestName(POIIndex poiIdx);
LatLon      getPointOfInterestPosition(POIIndex poiIdx);
OSMID       getPointOfInterestOSMNodeID(POIIndex poiIdx);




/*********************************************************************************
 * Natural features
 *
 * Natural features may be derived from OSM nodes, ways, or relations. The OSM 
 * entity type and OSM ID can be queried with the functions below to match 
 * features (by FeatureIndex) with layer 1 information.
 */

const std::string&  getFeatureName(FeatureIndex featureIdx);
FeatureType         getFeatureType(FeatureIndex featureIdx);
TypedOSMID          getFeatureOSMID(FeatureIndex featureIdx);
unsigned            getFeaturePointCount(FeatureIndex featureIdx);
LatLon              getFeaturePoint(FeatureIndex featureIdx, unsigned idx);
