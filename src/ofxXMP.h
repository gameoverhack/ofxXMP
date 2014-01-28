//
//  ofxXMP.h
//  emptyExample
//
//  Created by gameover on 27/11/13.
//
//

#ifndef _H_OFXXMP
#define _H_OFXXMP

#ifdef USE_BOOST_SERIALIZE
#include "SerializationUtils.h"
#endif

#include "ofMain.h"

// Must be defined to instantiate template classes
#define TXMP_STRING_TYPE std::string

// Must be defined to give access to XMPFiles
#define XMP_INCLUDE_XMPFILES 1

#ifdef TARGET_OSX
    #define MAC_ENV 1
#endif

// Ensure XMP templates are instantiated
#include "XMP.incl_cpp"

// Provide access to the API
#include "XMP.hpp"

static bool ofxXMPIsSetup = false;

class ofxXMPMarker;

class ofxXMP{
    
public:
    
    ofxXMP();
    ~ofxXMP();
    
    void setup();
    void clear();
    
    void loadXMP(string path);
    
    void dumpDynamicMetaData();
    
    bool listMarkers();
    
    ofxXMPMarker getMarker(string name);
    ofxXMPMarker getMarker(int index);
    
    int size();
    
    vector<ofxXMPMarker> getMarkers(string name);
    
    int getLastMarkerIndex(int frame, bool useDuration = false);
    int getNextMarkerIndex(int frame, bool useDuration = false);
    
    ofxXMPMarker getLastMarker(int frame, bool useDuration = false);
    ofxXMPMarker getNextMarker(int frame, bool useDuration = false);
    
    SXMPMeta & getMetaData();
    
    void setAllowDoubles(bool b);
    void setNormaliseMarkers(bool b);
    void setRemoveCarriageReturns(bool b);
    
protected:
    
    SXMPMeta meta;
    
    vector<ofxXMPMarker> markerVec;
    multimap<string, ofxXMPMarker> markerMap;
    
    bool bAllowDoubles;
    bool bNormaliseFrames;
    bool bRemoveCarriageReturns;
    
    int firstFrame;
    
#ifdef USE_BOOST_SERIALIZE
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        
        ar & BOOST_SERIALIZATION_NVP(markerVec);
        ar & BOOST_SERIALIZATION_NVP(markerMap);
        ar & BOOST_SERIALIZATION_NVP(bAllowDoubles);
        ar & BOOST_SERIALIZATION_NVP(bNormaliseFrames);
        ar & BOOST_SERIALIZATION_NVP(firstFrame);
        
    }
#endif
    
};

class ofxXMPMarker{
    
public:
    
    ofxXMPMarker(){
        clear();
    }
    
    ~ofxXMPMarker(){
        clear();
    }
    
    void setName(string markerName){
        name = markerName;
    }
    
    string getName(){
        return name;
    }
    
    void setStartTime(float time){
        assert(time >= 0.0f);
        startTime = time;
        startFrame = floor(startTime * timeBase);
        calculateDurationTime();
    }
    
    float getStartTime(){
        return startTime;
    }
    
    void setStartFrame(int frame){
        assert(frame >= 0);
        startFrame = frame;
        startTime = (float)startFrame / timeBase;
        calculateDurationFrames();
    }
    
    int getStartFrame(){
        return startFrame;
    }
    
    void setEndTime(float time){
        assert(time >= 0.0f);
        endTime = time;
        endFrame = floor(endTime * timeBase);
        calculateDurationTime();
    }
    
    float getEndTime(){
        return endTime;
    }
    
    void setEndFrame(int frame){
        assert(frame >= 0);
        endFrame = frame;
        endTime = (float)endFrame / timeBase;
        calculateDurationFrames();
    }
    
    int getEndFrame(){
        return endFrame;
    }
    
    void setDurationTime(float time){
        assert(time >= 0.0f);
        setEndTime(startTime + time);
    }
    
    float getDurationTime(){
        return durationTime;
    }
    
    void setDurationFrames(int frames){
        assert(frames >= 0);
        setEndFrame(startFrame + frames);
    }
    
    int getDurationFrames(){
        return durationFrames;
    }
    
    void setTimeBase(float framesPerSecond){
        assert(framesPerSecond > 0.0f);
        timeBase = framesPerSecond;
    }
    
    float getTimeBase(){
        return timeBase;
    }
    
    void setProbability(float prob){
        probability = prob;
    }
    
    float getProbability(){
        return probability;
    }
    
    void setSpeaker(string speakerName){
        if(speakerName != "Speaker Unknown") speaker = speakerName;
    }
    
    string getSpeaker(){
        return speaker;
    }
    
    void clear(){
        name = "";
        timeBase = 25;
        startTime = endTime = durationTime = 0.0f;
        startFrame = endFrame = durationFrames = 0;
        speaker = "";
        probability = 0.0f;
    }
    
    bool operator!=(const ofxXMPMarker &rhs){
        if(name != rhs.name ||
           timeBase != rhs.timeBase ||
           startFrame != rhs.startFrame ||
           startTime != rhs.startTime ||
           endFrame != rhs.endFrame ||
           endTime != rhs.endTime ||
           durationFrames != rhs.durationFrames ||
           durationTime != rhs.durationTime){
            return true;
        }else{
            return false;
        }
    }
    
    bool operator==(const ofxXMPMarker &rhs){
        if(name == rhs.name &&
           timeBase == rhs.timeBase &&
           startFrame == rhs.startFrame &&
           startTime == rhs.startTime &&
           endFrame == rhs.endFrame &&
           endTime == rhs.endTime &&
           durationFrames == rhs.durationFrames &&
           durationTime == rhs.durationTime){
            return true;
        }else{
            return false;
        }
    }
    
    friend ostream& operator<< (ostream &os, const ofxXMPMarker &m);
    
protected:
    
    void calculateDurationTime(){
        if(endTime >= startTime){
            durationTime = endTime - startTime;
        }else{
            endTime = startTime;
            durationTime = 0.0f;
        }
        durationFrames = floor(durationTime * timeBase);
    }
    
    void calculateDurationFrames(){
        if(endFrame >= startFrame){
            durationFrames = endFrame - startFrame;
        }else{
            endFrame = startFrame;
            durationFrames = 0;
        }
        durationTime = (float)durationFrames / timeBase;
    }
           
    string name;
    
    float timeBase;
    int startFrame;
    float startTime;
    int endFrame;
    float endTime;
    int durationFrames;
    float durationTime;
    
    // for speech
    string speaker;
    float probability;

#ifdef USE_BOOST_SERIALIZE
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        
        ar & BOOST_SERIALIZATION_NVP(name);
        ar & BOOST_SERIALIZATION_NVP(timeBase);
        ar & BOOST_SERIALIZATION_NVP(startFrame);
        ar & BOOST_SERIALIZATION_NVP(startTime);
        ar & BOOST_SERIALIZATION_NVP(endFrame);
        ar & BOOST_SERIALIZATION_NVP(endTime);
        ar & BOOST_SERIALIZATION_NVP(durationFrames);
        ar & BOOST_SERIALIZATION_NVP(durationTime);
        ar & BOOST_SERIALIZATION_NVP(speaker);
        ar & BOOST_SERIALIZATION_NVP(probability);
        
    }
#endif
    
};

inline ostream& operator<<(ostream& os, ofxXMPMarker &m){
    os << m.getName() << " " << m.getStartFrame() << " [" << m.getStartTime() << "] - " << m.getEndFrame() << " [" << m.getEndTime() << "] = " << m.getDurationFrames() << " [" << m.getDurationTime() << "] " << string(m.getProbability() != -1.0f ? ofToString(m.getProbability()) : "");
    return os;
}

static ofxXMPMarker NoMarker;
static vector<ofxXMPMarker> NoMarkers;
    
#endif
