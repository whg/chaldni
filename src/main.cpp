#include "ofMain.h"
#include "ofApp.h"

int main(int argc, char **argv ){

	ofSetupOpenGL(1000,800, OF_WINDOW);
    
    ofApp *app = new ofApp();
    
    if (argc >= 2) {
        app->dmxDevice = argv[1];
        cout << "setting dmxDevice to " << argv[1] << endl;
    }
    
    
	ofRunApp(app);

}
