/*
# Released under MIT License

Copyright (c) 2022 Héctor Hugo Avilés Arriaga.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>

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

volatile unsigned int width = 639;
volatile unsigned int height = 281;
cv::Mat orig_image(cv::Size(width, height), CV_8UC3);
cv::Rect myROI(0, 140, 639, 141); // (x,y, x + width, y + height)

    
void image_processing(void *){
// Define points that are used for generating bird's eye view. This was done by trial and error. Best to prepare sliders and configure for each use case. 
      cv::Point2f srcVertices[4];       
      // Funciona mejor todavía
      srcVertices[0] = cv::Point(*x1, *y1);
      srcVertices[1] = cv::Point(*x2, *y1);
      srcVertices[2] = cv::Point(*x3, *y2);
      srcVertices[3] = cv::Point(*x4, *y2);    

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

    cv::imshow("Display Image Gray", img);
    cv::waitKey(0);
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
    

int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    orig_image = cv::imread( argv[1], 1 );
    if ( !orig_image.data )
    {
        printf("No image data \n");
        return -1;
    }
    // Define the images that will be used first
    cv::Mat temp;
    cv::Mat orig;

    // Inicio para imágenes 640x480, x1 -90 x2 550 x3 600 x4 -190 y1 250 y2 300
    int x1 = 50;
    int x2 = 630;
    int x3 = 690;
    int x4 = -70;
    int y1 = 190;
    int y2 = 260;
  
    int kfd = 0;
    char c;
    struct termios cooked, raw;
    int sign = 1;
    
    orig = orig_image.clone();
    image_processing();  
    
    return 0;
    }
    
    
    
