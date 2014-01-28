//
//  ofxXMP.cpp
//  emptyExample
//
//  Created by gameover on 27/11/13.
//
//

#include "ofxXMP.h"

//--------------------------------------------------------------
ofxXMP::ofxXMP(){
    clear();
}

//--------------------------------------------------------------
ofxXMP::~ofxXMP(){
    clear();
}

//--------------------------------------------------------------
void ofxXMP::setup(){
    
    if(ofxXMPIsSetup) return;
    
    bAllowDoubles = bNormaliseFrames = bRemoveCarriageReturns = false;
    
    if(!SXMPMeta::Initialize()){
		ofLogError() << "Could not initialize XMP toolkit!";
		return;
	}else{
        ofLogVerbose() << "Initialised XMP toolkit";
    }
	
    XMP_OptionBits options = 0;
    
#if UNIX_ENV
    options |= kXMPFiles_ServerMode;
#endif
    
	// Must initialize SXMPFiles before we use it
	if(!SXMPFiles::Initialize(options)){
		ofLogError() << "Could not initialize XMP SXMPFiles.";
		return;
	}else{
        ofLogVerbose() << "Initialised XMP SXMPFiles";
        ofxXMPIsSetup = true;
    }
}

void ofxXMP::clear(){
    
    firstFrame = -1;
    
    // clear storage
    markerVec.clear();
    markerMap.clear();
    
    // Terminate the toolkit
	SXMPFiles::Terminate();
	SXMPMeta::Terminate();
    
}

//--------------------------------------------------------------
void ofxXMP::loadXMP(string path){
    
    clear();
    
    ofLogVerbose() << "Attempting to load XMP from: " << path;
    
    try{
		// Options to open the file with - read only and use a file handler
		XMP_OptionBits opts = kXMPFiles_OpenForRead | kXMPFiles_OpenUseSmartHandler;
        
		bool ok;
        
        SXMPMeta::Initialize();
        
		SXMPFiles myFile;
		std::string status = "";
        
		// First we try and open the file
		ok = myFile.OpenFile(path, kXMP_UnknownFile, opts);
		if(!ok){
			status += "No smart handler available for " + path + "\n";
			status += "Trying packet scanning.\n";
            
			// Now try using packet scanning
			opts = kXMPFiles_OpenForUpdate | kXMPFiles_OpenUsePacketScanning;
			ok = myFile.OpenFile(path, kXMP_UnknownFile, opts);
		}
        
        
		// If the file is open then read the metadata
		if(ok){
			ofLogVerbose() << status;
			ofLogVerbose() << path << " is opened successfully";
			// Create the xmp object and get the xmp data
			myFile.GetXMP(&meta);
        }else{
			ofLogError() << "Unable to open " << path;
		}
        
        myFile.CloseFile();
        
	}catch(XMP_Error & e){
		ofLogError() << "ERROR: " << e.GetErrMsg();
	}
    
}

//--------------------------------------------------------------
void ofxXMP::dumpDynamicMetaData(){
    SXMPIterator iter(meta, kXMP_NS_XMP_MM, (kXMP_IterJustLeafNodes));// | kXMP_IterJustLeafName));
    string schemaNS="", propPath="", propVal="";
    while(iter.Next(&schemaNS, &propPath, &propVal)){
        cout << propPath << " = " << propVal << endl;
    }
}

//--------------------------------------------------------------
bool ofxXMP::listMarkers(){
    
    if(markerVec.size() == 0){
        
        // iterate 'pantries'...
        int pantryCount = meta.CountArrayItems(kXMP_NS_XMP_MM, "Pantry");
        for(int pantry = 0; pantry < pantryCount; pantry++){
            
            // iterate tracks...
            string trackSearch = "Pantry[" + ofToString(pantry + 1) + "]/xmpDM:Tracks";
            int trackCount = meta.CountArrayItems(kXMP_NS_XMP_MM, trackSearch.c_str());
            for(int track = 0; track < trackCount; track++){
                
                // get the track name
                string trackName = "";
                string trackNameSearch = trackSearch + "[" + ofToString(track + 1) + "]/xmpDM:trackName";
                meta.GetProperty(kXMP_NS_XMP_MM , trackNameSearch.c_str(), &trackName, NULL);
                
                // get the frame rate for the track
                string frameRateString = "";
                string frameRateSearch = trackSearch + "[" + ofToString(track + 1) + "]/xmpDM:frameRate";
                meta.GetProperty(kXMP_NS_XMP_MM , frameRateSearch.c_str(), &frameRateString, NULL);
                
                frameRateString = frameRateString.substr(1, frameRateString.size());
                
                float frameRate = ofToFloat(frameRateString);
                                
                if(trackName == "layer_markers") continue; // only search for comp_markers && speech right now!!!
                if(!bAllowDoubles && markerVec.size() > 0) continue;
                
                ofLogVerbose() << "Listing markers for " << trackName << " # " << track << " in Pantry " << pantry << " using frame rate " << frameRate;
                
                // iterate markers...
                string markerSearch = trackSearch + "[" + ofToString(track + 1) + "]/xmpDM:markers";
                int markerCount = meta.CountArrayItems(kXMP_NS_XMP_MM, markerSearch.c_str());
                for(int marker = 0; marker < markerCount; marker++){
                    
                    string nameString = "";
                    string nameSearch = markerSearch + "[" + ofToString(marker + 1) + "]/xmpDM:name";
                    meta.GetProperty(kXMP_NS_XMP_MM , nameSearch.c_str(), &nameString, NULL);
                    
                    string commentString = "";
                    string commentSearch = markerSearch + "[" + ofToString(marker + 1) + "]/xmpDM:comment";
                    meta.GetProperty(kXMP_NS_XMP_MM , commentSearch.c_str(), &commentString, NULL);
                    
                    // use name or comment for marker name
                    if(nameString == "" && commentString != "") nameString = commentString;
                    
                    string startString = "";
                    string startTimeSearch = markerSearch + "[" + ofToString(marker + 1) + "]/xmpDM:startTime";
                    meta.GetProperty(kXMP_NS_XMP_MM , startTimeSearch.c_str(), &startString, NULL);
                    
                    string durationString = "";
                    string durationSearch = markerSearch + "[" + ofToString(marker + 1) + "]/xmpDM:duration";
                    meta.GetProperty(kXMP_NS_XMP_MM , durationSearch.c_str(), &durationString, NULL);
                    
                    string speakerString = "";
                    string speakerSearch = markerSearch + "[" + ofToString(marker + 1) + "]/xmpDM:speaker";
                    meta.GetProperty(kXMP_NS_XMP_MM , speakerSearch.c_str(), &speakerString, NULL);
                    
                    string probabilityString = "";
                    string probabilitySearch = markerSearch + "[" + ofToString(marker + 1) + "]/xmpDM:probability";
                    meta.GetProperty(kXMP_NS_XMP_MM , probabilitySearch.c_str(), &probabilityString, NULL);
                    
                    ofxXMPMarker m;
                    
                    if(trackName == "comp_markers") m.setTimeBase(floor(frameRate / 1000.0f));
                    if(trackName == "text transcription") m.setTimeBase(floor(frameRate));
                    
                    if(nameString != ""){
                        if(bRemoveCarriageReturns){
                            nameString.erase( std::remove(nameString.begin(), nameString.end(), '\r'), nameString.end() );
                            nameString.erase( std::remove(nameString.begin(), nameString.end(), '\n'), nameString.end() );
                        }
                        m.setName(nameString);
                    }
                    if(startString != ""){
                        float frame = ofToFloat(startString);
                        if(bNormaliseFrames){
                            if(firstFrame == -1) firstFrame = frame;
                            m.setStartTime((frame - firstFrame) / frameRate);
                        }else{
                            m.setStartTime(frame / frameRate);
                        }
                    }
                    if(durationString != "") m.setDurationTime(ofToFloat(durationString) / frameRate);
                    if(probabilityString != "") m.setProbability(ofToFloat(probabilityString) / 100.0f);
                    if(speakerString != "") m.setSpeaker(speakerString);
                    
//                    // hack to avoid duplicates
//                    if(!bAllowDoubles){
//                        ofxXMPMarker & mm = getMarker(m.getName());
//                        if(mm == m) continue;
//                    }
                    
                    // store the markers
                    markerVec.push_back(m);
                    markerMap.insert(pair<string, ofxXMPMarker>(m.getName(), m));
                    
                } // for(int marker = 0; marker < markerCount; marker++){
                
            } // for(int track = 0; track < trackCount; track++){
            
        } // for(int pantry = 0; pantry < pantryCount; pantry++){
        
    } // if(markerVector.size() == 0){
    
    if(markerVec.size() > 0){
        
//        for(int marker = 0; marker < markerVec.size(); marker++){
//            cout << markerVec[marker] << endl;
//        }
        
        // there are markers
        return true;
        
    }else{
        
        // there are no markers
        return false;
        
    }
    
}

//--------------------------------------------------------------
ofxXMPMarker ofxXMP::getMarker(int index){
    if(index < markerVec.size()){
        return markerVec[index];
    }
    return NoMarker;
}

//--------------------------------------------------------------
int ofxXMP::size(){
    return markerVec.size();
}

//--------------------------------------------------------------
ofxXMPMarker ofxXMP::getMarker(string name){
    for(int marker = 0; marker < markerVec.size(); marker++){
        ofxXMPMarker & m = markerVec[marker];
        if(m.getName() == name) return m;
    }
    return NoMarker;
}

//--------------------------------------------------------------
vector<ofxXMPMarker> ofxXMP::getMarkers(string name){
    vector<ofxXMPMarker> m = NoMarkers;
    pair <multimap<string, ofxXMPMarker>::iterator, multimap<string, ofxXMPMarker>::iterator> range = markerMap.equal_range(name);
    for (multimap<string, ofxXMPMarker>::iterator it = range.first; it != range.second; ++it){
        m.push_back(it->second);
    }
    return m;
}

//--------------------------------------------------------------
int ofxXMP::getLastMarkerIndex(int frame, bool useDuration){
    for(int marker = 0; marker < markerVec.size(); marker++){
        ofxXMPMarker & m = markerVec[marker];
        float durationFrames = 0;
        if(m.getDurationFrames() > 0 && useDuration){
            durationFrames = m.getDurationFrames();
        }else{
            if(marker < markerVec.size() - 1){
                durationFrames = markerVec[marker + 1].getStartFrame() - m.getStartFrame();
            }else{
                durationFrames = INFINITY;
            }
        }
        if(m.getStartFrame() <= frame && frame < m.getStartFrame() + durationFrames) return marker;
    }
    return -1;
}

//--------------------------------------------------------------
int ofxXMP::getNextMarkerIndex(int frame, bool useDuration){
    int index = getLastMarkerIndex(frame, useDuration);
    if(index + 1 < markerVec.size()) return index + 1;
    return index; // should it be cyclic ie., return 0?
}

//--------------------------------------------------------------
ofxXMPMarker ofxXMP::getLastMarker(int frame, bool useDuration){
    int index = getLastMarkerIndex(frame, useDuration);
    if(index != -1) return markerVec[index];
    return NoMarker; // should it be cyclic ie., return markerVec.size() - 1?
}

//--------------------------------------------------------------
ofxXMPMarker ofxXMP::getNextMarker(int frame, bool useDuration){
    int index = getNextMarkerIndex(frame, useDuration);
    if(index != -1) return markerVec[index];
    return NoMarker;
}

//--------------------------------------------------------------
void ofxXMP::setAllowDoubles(bool b){
    bAllowDoubles = b;
}

//--------------------------------------------------------------
void ofxXMP::setNormaliseMarkers(bool b){
    bNormaliseFrames = b;
}

//--------------------------------------------------------------
void ofxXMP::setRemoveCarriageReturns(bool b){
    bRemoveCarriageReturns = b;
}