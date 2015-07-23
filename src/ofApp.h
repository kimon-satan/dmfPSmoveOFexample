#pragma once

#include "ofMain.h"
#include <dmf_device.hpp>
#include <dmf_devicehandler.hpp>

#include <unistd.h>
#include <stdio.h>

using namespace DMF;

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void reallocateDevices();
    
        DeviceHandler mDeviceHandler;
        vector<DeviceDataMap * > mpInputMaps, mpOutputMaps;
        DeviceDataMap * mpHostData;
        DeviceDataMap * mpHostOutputData;
        string mDeviceInfo;
        u32 mNumDevices;
};
