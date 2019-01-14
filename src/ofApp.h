#pragma pack(16)


#include "ofMain.h"
#include "ofxMaxim.h"

class ofApp : public ofBaseApp{
    
public:
    void        setup();
    void        update();
    void        draw();
    
    void        keyPressed  (int key);
    void        keyReleased(int key);
    void        mouseMoved(int x, int y );
    void        mouseDragged(int x, int y, int button);
    void        mousePressed(int x, int y, int button);
    void        mouseReleased(int x, int y, int button);
    void        windowResized(int w, int h);
    void        dragEvent(ofDragInfo dragInfo);
    void        gotMessage(ofMessage msg);
    
    void        audioOut (float * output, int bufferSize, int nChannels);
    
    float       getTileAverage(const ofPixels& );            // returns the average difference in that tile
    ofPixels    makeTile(ofPixels&, int _row, int _tile);     // divides the image into tiles
    void        setGradients(const ofPixels , size_t count);
    

    
    double      outputs[2], frq1, frq2, frq3, frq4, frq5, frq6;
    
    maxiFilter  filter1, filter2, filter3, filter4;
    maxiOsc     osc1, osc2, osc3, osc4, LFO1, LFO2;
    maxiEnv     env1;
    bool        mute;

    int		    initialBufferSize; /* buffer size */
    int		    sampleRate;
    
    ofVideoPlayer 		myMovie;
    ofVideoGrabber		myGrabber;
    
    ofPixels            pixels;
    ofPixels            tile, xGradient, yGradient, xyGradients, angles, mags;
    ofPixels            pixelout, lastPixels;
    
    vector<float>       tileAvs, angleAvs;
    deque<ofPixels>     xyGradVec, angleVec, magVec;
    deque<ofPixels>     grid;
    
    ofTexture           myTexture, tileTexture, gradTex;

    
    int                 width, height, tileHeight, tileWidth, tileCount, gridSize, xTiles, yTiles;
    float               graphStep;
    
    array<int, 9>       bin;        // intended for histogram of movement

    
    

};
