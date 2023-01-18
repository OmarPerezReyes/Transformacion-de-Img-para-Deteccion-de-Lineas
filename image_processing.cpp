/*
# Released under MIT License

Copyright (c) 2022 Héctor Hugo Avilés Arriaga.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

// Demo: https://janhalozan.com/2019/06/01/lane-detector/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h> /* for strcpy() */
#include <time.h>
#include <iostream>
#include <atomic>

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "image_processing.hpp"
#include "glwindow.hpp"

#define KEYCODE_I 0x69
#define KEYCODE_J 0x6a
#define KEYCODE_K 0x6b
#define KEYCODE_L 0x6c
#define KEYCODE_Q 0x71
#define KEYCODE_Z 0x7a
#define KEYCODE_W 0x77
#define KEYCODE_X 0x78
#define KEYCODE_E 0x65
#define KEYCODE_C 0x63
#define KEYCODE_U 0x75
#define KEYCODE_O 0x6F
#define KEYCODE_M 0x6d
#define KEYCODE_R 0x72
#define KEYCODE_V 0x76
#define KEYCODE_T 0x74
#define KEYCODE_B 0x62

#define KEYCODE_COMMA 0x2c
#define KEYCODE_PERIOD 0x2e


// Global
// HHAA: This is the function pointer type for window position callbacks. A window position callback function has the following signature: void callback_name(GLFWwindow* window, int xpos, int ypos)
volatile unsigned int width = 639;
volatile unsigned int height = 281;
unsigned int fps = 30;
extern volatile bool die;

// Shared OpenCV images
cv::Mat orig_image(cv::Size(width, height), CV_8UC3);

pthread_mutex_t img_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t img_cond = PTHREAD_COND_INITIALIZER;


//
int mssleep(long milliseconds){

  struct timespec rem;
  struct timespec req = {
        (int)(milliseconds / 1000), /*secs (Must be Non-Negative)*/
        (milliseconds % 1000) * 1000000 /*nano (Must be in range of 0 to 999999999)*/
  };

  return nanosleep(&req, &rem); 

}

// HHAA

// Revisar: https://www.youtube.com/watch?v=7M99dovhx8M
void *image_processing_threadfunc(void *){

    // Define the images that will be used first
    cv::Mat temp;
    cv::Mat orig;

    // Inicio para imágenes 640x480
      // x1 -90 x2 550 x3 600 x4 -190 y1 250 y2 300
      int x1 = 50;
      int x2 = 630;
      int x3 = 690;
      int x4 = -70;
      int y1 = 190;
      int y2 = 260;
  

    int kfd = 0;
    char c;
    struct termios cooked, raw;

    // get the console in raw mode
    tcgetattr(kfd, &cooked);
    memcpy(&raw, &cooked, sizeof(struct termios));
    raw.c_lflag &=~ (ICANON | ECHO);
    raw.c_cc[VEOL] = 1;
    raw.c_cc[VEOF] = 2;
    tcsetattr(kfd, TCSANOW, &raw);

    puts("Reading from keyboard");
    puts("---------------------------");
    puts("Moving around:");
    puts("   u    i    o");
    puts("   j    k    l");
    puts("   m    ,    .");
    puts("");
    puts("q/z : increase/decrease max speeds by 10%");
    puts("w/x : increase/decrease only linear speed by 10%");
    puts("e/c : increase/decrease only angular speed by 10%");
    puts("");
    puts("anything else : stop");
    puts("---------------------------");
       
    // The definition of a ROI allows to proccess the ROI as an independent image, but any modification to the ROI will be reflected into the original image.     
    cv::Rect myROI(0, 140, 639, 141); // (x,y, x + width, y + height)
        
    glfwNamedWindow("Orig"); 
    glfwNamedWindow("DstGrey");             
           
    int i = 0;
    int sign = 1;
    while (!die) {
      int mutex_locked = pthread_mutex_lock(&img_mutex); 
      assert(mutex_locked == 0);
                         
        int cond_wait = pthread_cond_wait(&img_cond, &img_mutex); 
        assert(cond_wait == 0);
        if (die) break;
                    
        orig = orig_image.clone();
      int mutex_unlocked = pthread_mutex_unlock(&img_mutex); 
      assert(mutex_unlocked == 0);
      
      //orig = temp(myROI);

      // Define points that are used for generating bird's eye view. This was done by trial and error. Best to prepare sliders and configure for each use case. 
      cv::Point2f srcVertices[4];       
      // Funciona mejor todavía
      srcVertices[0] = cv::Point(x1, y1);
      srcVertices[1] = cv::Point(x2, y1);
      srcVertices[2] = cv::Point(x3, y2);
      srcVertices[3] = cv::Point(x4, y2);    

      // Destination vertices. Output is 640 by 480px 
      
      cv::Point2f dstVertices[4];
      dstVertices[0] = cv::Point(0, 0);
      dstVertices[1] = cv::Point(639, 0);
      dstVertices[2] = cv::Point(639, 281);
      dstVertices[3] = cv::Point(0, 281);

      // Prepare matrix for transform and get the warped image 
      cv::Mat perspectiveMatrix = getPerspectiveTransform(srcVertices, dstVertices);
      cv::Mat dst(281, 639, CV_8UC3); //Destination for warped image 
//For transforming back into original image space 
      cv::Mat invertedPerspectiveMatrix;
      cv::invert(perspectiveMatrix, invertedPerspectiveMatrix);

      cv::warpPerspective(orig, dst, perspectiveMatrix, dst.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);

   
      //Convert to gray 
      cv::Mat img;
      cv::cvtColor(dst, img, cv::COLOR_RGB2GRAY);

/*

      //Extract yellow and white info 
      cv::Mat maskYellow, maskWhite;

      cv::inRange(img, cv::Scalar(20, 100, 100), cv::Scalar(30, 255, 255), maskYellow);
      cv::inRange(img, cv::Scalar(150, 150, 150), cv::Scalar(255, 255, 255), maskWhite);
      cv::Mat mask, processed;
      cv::bitwise_or(maskYellow, maskWhite, mask); //Combine the two masks 
      cv::bitwise_and(img, mask, processed); //Extract what matches   

      //Blur the image a bit so that gaps are smoother 
      const cv::Size kernelSize = cv::Size(9, 9);
      cv::GaussianBlur(processed, processed, kernelSize, 0);

      //Try to fill the gaps 
      cv::Mat kernel = cv::Mat::ones(15, 15, CV_8U);
      cv::dilate(processed, processed, kernel);
      cv::erode(processed, processed, kernel);
      cv::morphologyEx(processed, processed, cv::MORPH_CLOSE, kernel);

      //Keep only what's above 150 value, other is then black 
      const int thresholdVal = 150;
      cv::threshold(processed, processed, thresholdVal, 255, cv::THRESH_BINARY);

      std::vector<cv::Point2f> pts = slidingWindow(processed, cv::Rect(0, 420, 120, 60));

      std::vector<cv::Point> allPts; //Used for the end polygon at the end. 
      std::vector<cv::Point2f> outPts;
      cv::perspectiveTransform(pts, outPts, invertedPerspectiveMatrix); //Transform points back into original image space 
//Draw the points onto the out image 
      for (int i = 0; i < outPts.size() - 1; ++i)
      {
       cv::line(orig, outPts[i], outPts[i + 1], cv::Scalar(255, 0, 0), 3);
       allPts.push_back(cv::Point(outPts[i].x, outPts[i].y));
      }

      allPts.push_back(cv::Point(outPts[outPts.size() - 1].x, outPts[outPts.size() - 1].y));

      cv::Mat out;
      cv::cvtColor(processed, out, cv::COLOR_GRAY2BGR); //Conver the processing image to color so that we can visualise the lines 
      for (int i = 0; i < pts.size() - 1; ++i) //Draw a line on the processed image     
      cv::line(out, pts[i], pts[i + 1], cv::Scalar(255, 0, 0));

//Sliding window for the right side 
//      pts = slidingWindow(processed, cv::Rect(520, 420, 120, 60));
      // pts = slidingWindow(processed, cv::Rect(0, 200, 639, 279));
//      perspectiveTransform(pts, outPts, invertedPerspectiveMatrix);
/*
//Draw the other lane and append points 
      for (int i = 0; i < outPts.size() - 1; ++i)
      {
        cv::line(orig, outPts[i], outPts[i + 1], cv::Scalar(0, 0, 255), 3);
        allPts.push_back(cv::Point(outPts[outPts.size() - i - 1].x, outPts[outPts.size() - i - 1].y));
      }

      allPts.push_back(cv::Point(outPts[0].x - (outPts.size() - 1) , outPts[0].y));

      for (int i = 0; i < pts.size() - 1; ++i)
        cv::line(out, pts[i], pts[i + 1], cv::Scalar(0, 0, 255));

      //Create a green-ish overlay 
      std::vector<std::vector<cv::Point>> arr;
      arr.push_back(allPts);
      cv::Mat overlay = cv::Mat::zeros(orig.size(), orig.type());
      cv::fillPoly(overlay, arr, cv::Scalar(0, 255, 100));
      cv::addWeighted(orig, 1, overlay, 0.5, 0, orig); //Ov

     // cv::line(image_roi, cv::Point(0,0), cv::Point(149,299), cv::Scalar(255,0,0), 2, 8);
*/
      glfwImageShow("Orig", orig);
      glfwImageShow("DstGrey", img);

        // get the next event from the keyboard
        if(read(kfd, &c, 1) < 0)
        {
          perror("read():");
          exit(-1);
        }

        switch(c)  
        {
            case KEYCODE_I:
              sign *= -1;
            case KEYCODE_Q:
              x1 = x1 + sign * 10;
            break;
            case KEYCODE_W:
              x2 = x2 + sign * 10;
            break;
            case KEYCODE_E:
              x3 = x3 + sign * 10;
            break;
            case KEYCODE_R:
              x4 = x4 + sign * 10;
            break;
            case KEYCODE_COMMA:
              y1 = y1 + sign * 10;
            break;
            case KEYCODE_PERIOD:
              y2 = y2 + sign * 10;
            break;
            case KEYCODE_T:
              die = true;
            break;
        }


      std::cout << "x1 " << x1;
      std::cout << " x2 " << x2;
      std::cout << " x3 " << x3;
      std::cout << " x4 " << x4;
      std::cout << " y1 " << y1;
      std::cout << " y2 " << y2;
      std::cout << " sign " << sign << std::endl;
      std::cout.flush();     

           
    }
    
    int mutex_unlocked = pthread_mutex_unlock(&img_mutex); 
    assert(mutex_unlocked == 0);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &cooked);
    std::cout << "Image processing is about to die " << std::endl;
    std::cout.flush();    
        
 return NULL;

}


// HHAA
void *image_capture_threadfunc(void *){



    while (!die) { 
      int mutex_locked = pthread_mutex_lock(&img_mutex); 
      assert(mutex_locked == 0);

        orig_image = cv::imread("./test.jpg");  
        if (orig_image.empty()){
          std::cout <<  "Could not open or find the image" << std::endl;
          std::cout.flush(); 
          die = true;        
          break;
        }      
        
        int mutex_cond = pthread_cond_signal(&img_cond); 
        assert(mutex_cond == 0);   
         
         
      int mutex_unlocked = pthread_mutex_unlock(&img_mutex); 
      assert(mutex_unlocked == 0);  
      // Required to allow the execution of the processing thread
                        

      mssleep(50);
    }
    
    int mutex_cond = pthread_cond_signal(&img_cond); 
    assert(mutex_cond == 0);        
    
    int mutex_unlocked = pthread_mutex_unlock(&img_mutex); 
    assert(mutex_unlocked == 0);
    
    std::cout << "Image capture is about to die " << std::endl;
    std::cout.flush();        

 return NULL;

}


std::vector<cv::Point2f> slidingWindow(cv::Mat image, cv::Rect window)
{
    std::vector<cv::Point2f> points;
    const cv::Size imgSize = image.size();
    bool shouldBreak = false;

    while (true)
    {
        float currentX = window.x + window.width * 0.5f;
        
        cv::Mat roi = image(window); //Extract region of interest         
        //std::vector<cv::Point2f> locations;
        cv::Mat locations;   // output, locations of non-zero pixels 

        cv::findNonZero(roi, locations); //Get all non-black pixels. All are white in our case         

        float avgX = 0.0f;
        
        for (int i = 0; i < locations.elemSize(); ++i) //Calculate average X position         
        {
            float x = locations.at<cv::Point>(i).x;
            avgX += window.x + x;
        }

        avgX = locations.empty() ? currentX : avgX / locations.elemSize();
        
        cv::Point point(avgX, window.y + window.height * 0.5f);
        points.push_back(point);

        //Move the window up         +
        window.y -= window.height;
        
        //For the uppermost position         
        if (window.y < 0)
        {
            window.y = 0;
            shouldBreak = true;
        }
        
        //Move x position         
        window.x += (point.x - currentX);
        
        //Make sure the window doesn't overflow, we get an error if we try to get data outside the matrix         
        if (window.x < 0)
            window.x = 0;
        if (window.x + window.width >= imgSize.width)
            window.x = imgSize.width - window.width - 1;

        if (shouldBreak)
            break;
    }
    
    return points;
}



/*
std::vector<cv::Point2f> slidingWindow(cv::Mat image, cv::Rect window)
{
    std::vector<cv::Point2f> points;
    const cv::Size imgSize = image.size();
    bool shouldBreak = false;

    while (true)
    {
        float currentX = window.x + window.width * 0.5f;
        
        cv::Mat roi = image(window); //Extract region of interest         
        std::vector<cv::Point2f> locations;

        cv::findNonZero(roi, locations); //Get all non-black pixels. All are white in our case         

        float avgX = 0.0f;
        
        for (int i = 0; i < locations.size(); ++i) //Calculate average X position         
        {
            float x = locations[i].x;
            avgX += window.x + x;
        }

        avgX = locations.empty() ? currentX : avgX / locations.size();
        
        cv::Point point(avgX, window.y + window.height * 0.5f);
        points.push_back(point);

        //Move the window up         +
        window.y -= window.height;
        
        //For the uppermost position         
        if (window.y < 0)
        {
            window.y = 0;
            shouldBreak = true;
        }
        
        //Move x position         
        window.x += (point.x - currentX);
        
        //Make sure the window doesn't overflow, we get an error if we try to get data outside the matrix         
        if (window.x < 0)
            window.x = 0;
        if (window.x + window.width >= imgSize.width)
            window.x = imgSize.width - window.width - 1;

        if (shouldBreak)
            break;
    }
    
    return points;
}

*/

// Modificaciones: 1) se removió el while (!new_images) porque el cond_wait hace bloqueo del thread de procesamiento y el while no es necesario, 2) se eliminó la variable new_image del programa por el punto anterior, 3) se incluyó dar unlock y cond_signal después de salir del while del thread de captura para terminar el procesamiento y se incluyó unlocked al salir del while del procesamiento como salida limpia para cuando ya no haya más imágenes, 4) al no haber más imágenes en captura se incluye if( orig_image.empty(), se hace die=true y se sale del while, 5) se incluye if (die) break; en el procesamiento después del assert(cond_wait) para prevenir intento de procesamiento si ya se tecleó Ctrl-C y 6) en el thread de capture se movió pthread_cond_signal a antes de hacer el unlock. 



