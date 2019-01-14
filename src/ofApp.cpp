// this is a motion controlled synth - most of the pixel processing code is from Mick Grierson's class examples, bar the functions to do the tiling and averaging


#include "ofApp.h"


//-------------------------------------------------------------

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofBackground(0,0,0);
    
    initialBufferSize = 512;
    sampleRate = 44100;
    ofxMaxiSettings::setup(sampleRate, 2, initialBufferSize);
    mute = 1;
    frq1 = frq2 = frq3 = frq4 = frq5 = frq6 = 0.;
    
    env1.setAttack(200);env1.setDecay(10);
    env1.setSustain(100);env1.setRelease(300);
    
    ofSoundStreamSetup(2, 0,this, sampleRate, initialBufferSize, 4);
    
    myGrabber.listDevices();
    //myGrabber.setPixelFormat(ofPixelFormat::OF_PIXELS_RGB);
    
    myGrabber.initGrabber(320,240);
    myGrabber.setDesiredFrameRate(30);
    
    // to grid this into 8x8 pixel squares means 320/8 == 40 horizontal + 240/8 == 30 vertical
    
    // width = myMovie.getWidth();
    // height = myMovie.getHeight();
    
    
    width = myGrabber.getWidth();
    height = myGrabber.getHeight();
    myTexture.allocate(width,height,GL_RGB);
    
    pixelout.allocate(width, height, 1);
    lastPixels.allocate(width, height,3);
    
    
//    myMovie.load(ofToDataPath("IMG_7088.mov"));
//    ofPixelFormat format = myMovie.getPixelFormat();
//   cout << ofToString(format) << endl;
    
    
//    if (myMovie.isLoaded()){
    if (myGrabber.isInitialized()){
            pixels = myGrabber.getPixels();
//            myMovie.play();
    }
   // cout << myMovie.getDuration() << endl;
    
    
    xTiles = 8;    yTiles = 8;
    tileWidth   =   width  / xTiles; // 16x16 pixel tiles
    tileHeight  =   height / yTiles;
    gridSize    =   xTiles * yTiles;
    cout << "gridsize " << gridSize << endl;
    tileAvs.reserve(gridSize+1);// reserve the vector memory in advance, might be a bit quicker
    tileAvs.clear();
    
    
    // *** allocate memory for loads of ofPixels
    
    tileTexture.allocate(tileWidth, tileHeight, GL_LUMINANCE);
    gradTex.allocate    (tileWidth, tileHeight, GL_LUMINANCE);
    
    tile.allocate        (tileWidth, tileHeight, 1);             //the tile
    xGradient.allocate   (tileWidth, tileHeight, 1);             //array for xgradients
    yGradient.allocate   (tileWidth, tileHeight, 1);             //array for ygradients
    xyGradients.allocate (tileWidth, tileHeight, 1);
    angles.allocate      (tileWidth, tileHeight, 1);             //array for angles
    mags.allocate        (tileWidth, tileHeight, 1);             // magnitudes
    
    angleAvs.reserve  (gridSize);
//    magVec.reserve    (gridSize);
//    xyGradVec.reserve (gridSize);
    

    graphStep = width/(float)gridSize;
    
//    ofSoundStreamSetup(0,0,this, sampleRate, initialBufferSize, 4);/* Call this last ! */
}

//--------------------------------------------------------------
void ofApp::update(){
    
//    myMovie.update();
    myGrabber.update();

    if (myGrabber.isFrameNew()) {

        pixels = myGrabber.getPixels();           // If there's a new frame, get the pixels
        
        // frame difference on Red channel... motion detection from class examples code
        for (int i = 0; i < width; i++){
            for (int j = 0; j < height; j++) {
                
                pixelout[(j*width+i)]=abs((lastPixels[(j*width+i)*3+1])-(pixels[(j*width+i)*3+1]));
                
                lastPixels[(j*width+i)*3+1]=pixels[(j*width+i)*3+1];
            }
        }
        // end of new frame loop
        //  make tiles from the 1 channel frame-diffed image
        
        for (int _row=0; _row<yTiles;_row++){
            for (int _tileNum=0; _tileNum<xTiles; _tileNum++){
                
              grid.push_back(makeTile(pixelout, _row, _tileNum));     // makes tiles, and push them onto a deque
                if (grid.size()>gridSize) {                                 //  circular buffer of tiles.
                    grid.pop_front();
                }
            }
        }
// get the average difference for each tile in the grid and store them in a vector, calculate the gradients, angles andn magnitudes
 
      //  if (grid.size()>0){
            int cnt = 0;
            
            for (const ofPixels &tile : grid){              //loop through all the tiles in the grid deque
                setGradients(tile, cnt);                    // calculate all the HOG gradient stuff
                tileAvs[cnt] = getTileAverage(tile);        // average movement in the tile for graphs and audio
                cnt++;
                cnt=cnt % gridSize;
       
       // }
            
            angleVec.push_back(angles);                                         // angles onto a deque
            if (angleVec.size()>gridSize) angleVec.pop_front();
            
            magVec.push_back(mags);
            if (magVec.size()>gridSize) magVec.pop_front();                     //magnitudes
            
            xyGradVec.push_back(xyGradients);                                   //xy Gradients
            if (xyGradients.size()>gridSize) xyGradVec.pop_front();
            }
        
            // ****** Audio Stuff *******/
        
            frq1 = tileAvs[0];     // give freq1 a value based on movement top left
            frq2 = tileAvs[7];
            frq3 = tileAvs[24];
            frq4 = tileAvs[31];
            frq5 = tileAvs[56];
            frq6 = tileAvs[63];
        
        
        //    cout << frq1 << endl;// top right
        
        angleAvs[0] = getTileAverage(angleVec[0]);
        
        myTexture.allocate(pixelout);
        tileTexture.allocate(pixels);
        
        if (angleVec.size()>0){
            gradTex.allocate(angleVec[0]);
           // cout << "allocated gradtex" << endl;
        }
  
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofSetColor(255, 255, 255,255);
    ofDrawBitmapString(ofToString(ofGetFrameRate()), 370, 260);
    ofDrawBitmapString(ofToString(frq1), 370, 280);
    ofDrawBitmapString(ofToString(angleAvs[0]), 370, 300);


    myTexture.draw(0, 0, width, height);                // draw the frame diff'd image
    tileTexture.draw(width, 0, width, height);          // draw the camera input
    
 
    if (gradTex.isAllocated()){
        gradTex.draw(width, height, width, height);
       // cout << "drawing xyGrads" << endl;// draw the xY gradients
    }
    
   for (int i=0;i<grid.size();i++){                              //graph the chnage across the individual tiles, lopasses to slow it down bit
                            ofSetColor(tileAvs[i]*2,180-tileAvs[i],50,220);
                            ofDrawRectangle((float)i*graphStep, 480, 1, filter1.lopass(-tileAvs[i]*2, 0.4));
   }
   // cout << "angles" <<  << endl;
}

//--------------------------------------------------------------

float ofApp::getTileAverage(const ofPixels &tilePix){  // takes in a reference to the pixel tiles to avoid copying data.
                                                        //Can turn ofPixels char*'s into floats for maximillian
 
    size_t arrayLength = tileWidth *  tileHeight;
    
    float sum = 0;
    
    for (int i=0;i<arrayLength;i++){
        sum += tilePix[i];
    }
    
    return sum / arrayLength;       // just returns a value
}

//--------------------------------------------------------------

ofPixels ofApp::makeTile(ofPixels& imagePixels, int rowNum, int tileNum){
    
    size_t numChannels = imagePixels.getNumChannels();      // in case a colour image comes in
    
    size_t arrayWidth = imagePixels.getWidth()*numChannels; // total width of the frame in array elements
    size_t rowStride = arrayWidth * tileHeight;             // the number of index positions to jump to get to the first                            pixel in the next row of tiles.
    
    
// Loop through the image, make grid of tiles
  
    
    for (int i=0; i<tileHeight; i++){
        
        int tileCol = i*tileWidth;
        
        for (int j=0; j<tileWidth; j++){
        
        
        tile.allocate(tileWidth, tileHeight, 1);
        tile[tileCol+j] = imagePixels[(i*arrayWidth + rowStride*rowNum) + ((j+tileWidth*tileNum)*numChannels)];
        }
    }
    
    return tile;
   // grid.push_back(tile); // returns pixel values
}

//--------------------------------------------------------------
void ofApp::setGradients(const ofPixels tilePix, size_t _count){

    for (int i = 1; i < tileHeight-1;i++) {
        
        int collm=(i-1)*tileWidth;          // the row above..
        int coll=i*tileWidth;
        int collp=(i+1)*tileWidth;          // the row below
        
        
        for (int j = 1; j < tileWidth-1; j++) {     //basically class code
            
            xGradient[coll+j] = (((tilePix[(coll+(j-1))]) *-1) + tilePix[coll+(j+1)]) * 0.5; // this is a 'Sobel' kernel - picking out edges in the x axis

            yGradient[coll+j] =  (((tilePix[collm+j])*-1) + tilePix[collp+j]) * 0.5; // same technique in y axis
            
            xyGradients[coll+j] =  xGradient[coll+j]+yGradient[coll+j]; // the combined gradients
            
            mags[coll+j]      =  sqrt((xGradient[coll+j]*xGradient[coll+j])+(yGradient[coll+j]*yGradient[coll+j])); // Pythagoras - root of the summmed squares -gives the hypotenuse, ie magnitude
    
            angles[coll+j]    =  floor(180.0 * ((atan2((double)yGradient[coll+j], (double)xGradient[coll+j]))/PI));// the inverse tangent gives the angle, restrained to 180 d
        }
    }
}
        
//--------------------------------------------------------------
//void ofApp::binner(){
//    
//    // a very dumb way of combining the magnitude and angle data into a HOG histogram
//    //bins are at 20 degree intervals, 9 total for 180 degrees..

// i was having massive problems with baffling memory deallocation stuff in ofPixels.cpp, which meant i had to abandon this bit..
// The idea was to use the shape of things to influence the sound...
// Possible should have used openCV histCalc :)


//    for (int i=0; i<tileHeight*tileWidth;i++){
//        
//        if (angleVec[i][i]>20 && angleVec[i][i]<40){
//            bin[0] += angleVec[i][i] * magVec[i][i];
//    }
//    
//    
//    
//}
//--------------------------------------------------------------
void ofApp::audioOut (float * output, int bufferSize, int nChannels){
    
    
    for (int i = 0; i < bufferSize; i++){
      /*
         http://www.maximilian.strangeloop.co.uk
         
         //there are 64 averages of movement data in avsVec...Each frequency (frq) references a different part of the screen..
       */
        
        double envTrig = LFO2.phasor(20+frq2);
        
        double loresOut = filter3.lores(((osc4.sinewave(frq6+30)*15)*100), osc4.phasor(frq6)*200, 0.1);
        
        double filterOut = filter2.lopass(((frq1*10+150))*10, 0.0001);
        
       
        
        double chan3   =   osc3.sinewave(150+LFO1.sinewave(frq5)*10);
        
         double env =      env1.adsr(chan3, 1.);
        
        double  chan2  =   0;//osc2.sawn(frq2*10+50)*10;
        
        double  chan1   = osc1.sinewave(filterOut*envTrig)*10;
        
        
        //        sample=beat.play(0.5, 0, beat.length);
        //        sample=beats.bufferPlay(*blockpix,40,0,ofGetWidth()*ofGetHeight()*0.5);
        //        sample=(sample-0.5)*2;
        //
        //        mymix.stereo(filtered, outputs, 0.5);
        //      mymix.stereo(sample, outputs, 0.5);
        
        //(chan1/4 + chan2/4 +
        output[i*nChannels    ] = outputs[0] = (chan1*.25 +chan3)  *mute;
        output[i*nChannels + 1] = outputs[1] = (loresOut*.25) * mute;
    }
    
}
//--------------------------------------------------------------


void ofApp::keyPressed(int key){
    
    if (key == ' ') mute = !mute;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    if(key == 's') {
        
        myGrabber.videoSettings();
    } 
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
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
