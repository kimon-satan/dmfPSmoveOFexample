#include "ofApp.h"




using namespace DMF;


//--------------------------------------------------------------
void ofApp::setup(){


    //check the working directory
    //cout << ofFilePath::getCurrentWorkingDirectory() << endl;
    
    mDeviceHandler.Init();
    mDeviceHandler.LoadPlugins("../../../plugins"); //this is the path from "dmfExample/bin/dmfExampleDebug.app/Contents/MacOS"
    
    mNumDevices = 0;
    
    /*DeviceDataMap* pMaps = NULL;
    ReturnCode result = mDeviceHandler.GetDeviceInfo(&pMaps, numDevices);
    
    DeviceDataMap* pClientData = NULL;
    DeviceDataMap* pClientOutputData = NULL;
    
    /*ReturnCode result = Allocate(mpInputMap, &pClientData);
    
    assert(result == kOK);
    if(result != kOK)
    {
        printf("Failed to Allocate input data map for device %d\n", idx);
        return;
    }
    
    result = Allocate(mpOutputMap, &pClientOutputData); //NB this assumes that all devices have an output map and that they appear in the same order ?
    assert(result == kOK);
    if(result != kOK)
    {
        printf("Failed to Allocate output data map for device %d\n", idx);
        return;
    }
  
    
    assert(mpMaps[idx].mDeviceID == mpOutputMaps[idx].mDeviceID); */
}

//--------------------------------------------------------------
void ofApp::update(){
    
    //update the device handler
    
    mDeviceHandler.Update();
    
    //check for new devices once a second
    
    if(ofGetFrameNum()%60  == 0){
        
        DMF::u32 numDevices = 0;
        
        mpHostData = NULL;
        mpHostOutputData = NULL;
        
        mDeviceHandler.GetDeviceInfo(&mpHostData, numDevices);
        mDeviceHandler.GetDeviceOutputInfo(&mpHostOutputData, numDevices);

        if(mNumDevices != numDevices){
            mNumDevices = numDevices;
            reallocateDevices();
        }
            
    }
    
    mDeviceInfo = "Device Info : \n\n";
 
    
    for(int i = 0; i < mNumDevices; i++){
        
        mDeviceInfo += "device ID : " + ofToString(mpInputMaps[i]->mDeviceID) + "\n";
        mDeviceInfo +=  "device Name : " + mpInputMaps[i]->mDeviceName + "\n";
        mDeviceInfo +=  "device type: " + mpInputMaps[i]->mDeviceType + "\n\n";
        
        ///////////////////////////////////////////////////////////////////
        /*
         This is an important point ... we work with a copy of the data not the actual data
         */
        
        // Copy from device memory to client memory
        mDeviceHandler.Update(mpInputMaps[i]->mDeviceID, mpInputMaps[i]);
        //Copy from client memory to device memory
        mDeviceHandler.UpdateOutput(mpOutputMaps[i]->mDeviceID, mpOutputMaps[i]);
        
        //////////////////////////////////////////////////////////////////////////////
        
        //all the data is stored in a custom databuffer at index 0
        
        for(int j = 0; j < mpInputMaps[i]->mCustomBuffer[0].mNumDataBuffers; j++)
        {
            DataBufferDescriptor descr =  mpInputMaps[i]->mCustomBuffer[0].mDataBuffer[j].mDescriptor;
            
            mDeviceInfo +=  descr.mName + ": ";
        
            f32 * fVal = NULL;
            ubyte * ubVal = NULL;
            u32 * uiVal = NULL;
            
            if(descr.mFormat == kDataFormatFloat){
            
                GetDataPtr(mpInputMaps[i]->mCustomBuffer[0].mDataBuffer[j], &fVal, descr.mID, descr.mFormat);
            
                for(int k = 0; k < descr.mNumElements; k++){
                    mDeviceInfo += ofToString(fVal[k],2) + " , ";
                }
            }
            
            if(descr.mFormat == kDataFormatUByte){
                
                GetDataPtr(mpInputMaps[i]->mCustomBuffer[0].mDataBuffer[j], &ubVal, descr.mID, descr.mFormat);
                
                for(int k = 0; k < descr.mNumElements; k++){
                    mDeviceInfo += ofToString(ubVal[k],2) + " , ";
                }
            }
            
            if(descr.mFormat == kDataFormatUInt){
                
                GetDataPtr(mpInputMaps[i]->mCustomBuffer[0].mDataBuffer[j], &uiVal, descr.mID, descr.mFormat);
                
                for(int k = 0; k < descr.mNumElements; k++){
                    mDeviceInfo += ofToString(uiVal[k]) + " , ";
                }
            }
            
            mDeviceInfo += "\n";
            
           
            
            
        }
        
         //now lets play with the outputs
        
        for(int j = 0; j < mpOutputMaps[i]->mCustomBuffer[0].mNumDataBuffers; j++)
        {
            DataBufferDescriptor descr =  mpOutputMaps[i]->mCustomBuffer[0].mDataBuffer[j].mDescriptor;
            
            
            mDeviceInfo +=  descr.mName + ": ";
            
            f32 * fVal = NULL;
            ubyte * ubVal = NULL;
            u32 * uiVal = NULL;
            
            if(descr.mFormat == kDataFormatFloat){
                
                GetDataPtr(mpOutputMaps[i]->mCustomBuffer[0].mDataBuffer[j], &fVal, descr.mID, descr.mFormat);
                
                for(int k = 0; k < descr.mNumElements; k++){
                    
                    if(descr.mName == "LedIntensity"){
                        fVal[0] = (sin(ofGetElapsedTimef()) + 1)/2.0;
                    }
                    
                    mDeviceInfo += ofToString(fVal[k],2) + " , ";
                }
            }
            
            if(descr.mFormat == kDataFormatUByte){
                
                GetDataPtr(mpOutputMaps[i]->mCustomBuffer[0].mDataBuffer[j], &ubVal, descr.mID, descr.mFormat);
                
                if(descr.mName == "LedVector"){
                    for(int col = 0; col < 3; col ++){
                        ubVal[col] = (sin(ofGetElapsedTimef() * (col + 1)) + 1)/2.0 * 255;
                    }
                }
                
                if(descr.mName == "Rumble"){
                    *ubVal = ofGetFrameNum()%20 <= 5 ? 255 : 0;
                }
                
                for(int k = 0; k < descr.mNumElements; k++){
                    mDeviceInfo += ofToString((int)ubVal[k],2) + " , ";
                }

            }
            
            if(descr.mFormat == kDataFormatUInt){
                
                GetDataPtr(mpOutputMaps[i]->mCustomBuffer[0].mDataBuffer[j], &uiVal, descr.mID, descr.mFormat);
                
                for(int k = 0; k < descr.mNumElements; k++){
                    mDeviceInfo += ofToString(uiVal[k]) + " , ";
                }
            }
            
            mDeviceInfo += "\n";
        }
        
        
        mDeviceInfo += "\n////////////////////////////////////////\n\n";
        
        
        
    }
    
    
    
   

}

void ofApp::reallocateDevices(){
    
    cout << "num devices has change to : " << ofToString(mNumDevices) << endl;
    
    mpInputMaps.clear();
    mpOutputMaps.clear();
    
    
    
    //Allocate a data copy for the all the devices
    
    for(int i = 0; i < mNumDevices; i++){
        
        DeviceDataMap * pClientData = NULL;
        DeviceDataMap * pClientOutputData = NULL;
        
        ReturnCode result = Allocate(mpHostData[i], &pClientData);
        
        if(result != kOK)
        {
            printf("Failed to Allocate output data map for device %d\n", i);
            return;
        }
        
        result = Allocate(mpHostOutputData[i], &pClientOutputData);
        
        if(result != kOK)
        {
            printf("Failed to Allocate output data map for device %d\n", i);
            return;
        }
        
        mpInputMaps.push_back(pClientData);
        mpOutputMaps.push_back(pClientOutputData);
        
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(75);
    ofSetColor(255);
    ofDrawBitmapString(mDeviceInfo, 20,20);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}