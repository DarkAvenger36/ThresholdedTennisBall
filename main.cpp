#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include <stdio.h>
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

//--------------IMPOSTAZIONI------------

//imposto la risoluzione di acquisizione
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

//valori preimpostati per tracciare la pallina, cambiabili con le trackbar
int H_MIN = 26;
int S_MIN = 75;
int V_MIN = 67;
int H_MAX = 256;
int S_MAX = 256;
int V_MAX = 256;

//numero massimo di oggetti rilevabili
int MAXOBJECT=10;

//finestre
const string mainGui="Immagine acquisita";
const string thresholdWindow="Immagine rilevata";
const string settingWindow="Imposta soglia";
const string blurWindow="Immagine con filtro Blur";

//action listener per gli slider -> INUTILISSIMO MA NECESSARIO
void onTrackbarSlide(int, void*){
        //non c'è bisogno di fare nulla        
}

//metodo che crea gli slider
void createSlider(){
        namedWindow(settingWindow,0);
        
        //metodo che crea le trackbar(label, finestra, valore da cambiare, valore massimo,action listener)
        createTrackbar("H-min",settingWindow, &H_MIN, 256, onTrackbarSlide);
        createTrackbar("S-min",settingWindow, &S_MIN, 256,onTrackbarSlide);
        createTrackbar("V-min",settingWindow, &V_MIN, 256,onTrackbarSlide);
        createTrackbar("H-max",settingWindow, &H_MAX, 256, onTrackbarSlide);
        createTrackbar("S-max",settingWindow, &S_MAX, 256,onTrackbarSlide);
        createTrackbar("V-max",settingWindow, &V_MAX, 256,onTrackbarSlide);
}

void plotVerticalLine(Mat img){
	int width1=FRAME_WIDTH/3;
	int width2=FRAME_WIDTH*2/3;
	int height1=FRAME_HEIGHT/3;
	int height2=FRAME_HEIGHT*2/3;
	//linee orizzontali
	cv::line(img,cv::Point(0,height1),cv::Point(FRAME_WIDTH,height1),cv::Scalar(0,0,200),3,CV_AA);
	cv::line(img,cv::Point(0,height2),cv::Point(FRAME_WIDTH,height2),cv::Scalar(0,0,200),3,CV_AA);
	//line verticali
	cv::line(img,cv::Point(width1,0),cv::Point(width1,FRAME_HEIGHT),cv::Scalar(0,0,200),3,CV_AA);
	cv::line(img,cv::Point(width2,0),cv::Point(width2,FRAME_HEIGHT),cv::Scalar(0,0,200),3,CV_AA);
}


int main(int argc,char* argv[]){
        
        //avvio della videocamera
        VideoCapture capture;
        
        //0--> webcam default (interna)
        //1--> webcam esterna
        capture.open(0);
        
        //matrice su cui verranno salvati i frame catturati
        Mat cameraFeed;

        //matrice su cui salvo l'immagine HSV
        Mat hsvFrame;
        
        //matrice su cui salvo l'immagine filtrata
        Mat thresholded;
        
        //matrice per mostrare l'immagine con il filtro blur
        Mat frameBlur;
        
        //matrice per Hough Transform
        Mat Hough;
        vector<Vec3f> circles;
        
        //imposto la dimensione dei frame da catturare
        capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
        capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
        

        //creo la finestra con gli slider
        createSlider();
        
        while(1){
                //catturo un frame della webcam
                capture.read(cameraFeed);

                //cambio lo spazio dei colori RGB-->HSV
                //cvtColor(sorgente, destinazione, operazione)
                cvtColor(cameraFeed,hsvFrame,COLOR_BGR2HSV);
                
                //filtro hsvFrame cercando solo un determinato range di colori
                //void inRange(InputArray src, InputArray lowerbound, InputArray upperbound, OutputArray dst)
                inRange(hsvFrame, Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),thresholded);
                
                //--------applico l'erosione e la dilatazione---------
                //questa matrice  indica che i 3x3 pixel intorno ad ogni punto da analizzare sono un rettangolo
                //in pratica per ogni pixel erodo il rettangolo 3x3 intorno 
                //IN PRATICA: con l'erosione rimuovo tutti i puntini piccoli che compaiono riducendo il rumore
                Mat rectErosione = getStructuringElement(MORPH_RECT,Size(3,3));
                erode(thresholded, thresholded,rectErosione);
                erode(thresholded, thresholded,rectErosione);
				erode(thresholded, thresholded,rectErosione);
                
                //dilato ogni pixel rilevato in un rect 8x8
                Mat rectDilataz = getStructuringElement( MORPH_RECT,Size(8,8));
                dilate(thresholded, thresholded, rectDilataz);
                dilate(thresholded, thresholded, rectDilataz);
				dilate(thresholded, thresholded, rectDilataz);

				//applico il gaussian blur
                //medianBlur(cameraFeed,frameBlur,5);
				GaussianBlur(thresholded,thresholded, Size(11,11),2,2);
                
                //Hough Transform
                HoughCircles(thresholded, circles, CV_HOUGH_GRADIENT, 2, thresholded.rows/4, 100, 40, 10, 120 );
                
                //Se il numero di cerchi rilevati è minore del massimo allora li disegno
                std::vector<int>::size_type numCircles = circles.size();
                //cerco la posizione massima
                int j=0;
                int max=0;
                for(int i=0;i<numCircles;i++){
					if(max<cvRound(circles[i][2])){
						max=cvRound(circles[i][2]);
						j=i;
					}
				}
				printf("max= %d \n",max);
				printf("j= %d \n",j);
				
                if(numCircles<MAXOBJECT){
                  for( int i = 0; i < numCircles; i++ )
                  {
                      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
                      int radius = cvRound(circles[i][2]);
                      if(i==j){
						  // circle center
						  circle( cameraFeed, center, 3, Scalar(0,255,0), -1, 8, 0 );
						  // circle outline
						  circle( cameraFeed, center, radius, Scalar(0,0,255), 3, 8, 0 );
					  }else{
						  						  // circle center
						  circle( cameraFeed, center, 3, Scalar(255,255,0), -1, 8, 0 );
						  // circle outline
						  circle( cameraFeed, center, radius, Scalar(255,0,255), 3, 8, 0 );
						  
					  }
                   }
			   }else{
				   printf("troppi oggetti rilevati! Eliminare il rumore! \n");
			   }
			   
			   //Visualizzo barre verticali sull'immagine
			   plotVerticalLine(cameraFeed);

                                
                //visualizzo su mainGui il frame originale
                imshow(mainGui,cameraFeed);
                
                //visualizzo su thresholdWindow l'immagine filtrata
                imshow(thresholdWindow,thresholded);
                //visualizzo su blurWindow l'immagine con il filtro blur
                //imshow(blurWindow,frameBlur);

                
                
                //premi esc per uscire oppure attendi il tempo di attesa prima di passare al frame successivo
                char c=waitKey(500);
                if(c == 27) break;
                }
        
        
        return(0);
}
