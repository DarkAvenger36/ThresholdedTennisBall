#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

int main(int argc, char** argv){

	cvNamedWindow("Test Video",0);
	cvResizeWindow("Test Video",640,480);
	
	//capture by the only webcam
	CvCapture* capture=cvCreateCameraCapture(-1);
	IplImage* frame;
	
	
	while(1){
	
		frame = cvQueryFrame(capture);
		
		//if the video finish or the cam is disconnected 
		if(!frame) break;
		
		cvShowImage("Test Video",frame);
		char c = cvWaitKey(33);
		
		//type "esc" to exit
		if(c==27) break;
	}
	
	//c++ haven't the garbage collector (Thomas said)
	cvReleaseCapture(&capture);
	cvDestroyWindow("Test Video");
	
	return 0;
}
