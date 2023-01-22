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
#define KEYCODE_K 0x6b
#define KEYCODE_Q 0x71
#define KEYCODE_W 0x77
#define KEYCODE_E 0x65
#define KEYCODE_R 0x72
#define KEYCODE_COMMA 0x2c //coma
#define KEYCODE_PERIOD 0x2e //punto

bool die = false;
bool first_time = true;
int x1;
int x2;
int x3;
int x4;
int y_1;
int y2;
int kfd;   
char c;
int sign;
struct termios cooked, raw;
    
cv::Point2f srcVertices[4];
cv::Point2f dstVertices[4];
                
volatile unsigned int width = 639;
volatile unsigned int height = 281;

cv::Mat orig_image(cv::Size(width, height), CV_8UC3);
cv::Rect myROI(0, 140, 639, 141); // (x,y, x + width, y + height)
cv::Mat temp;
cv::Mat orig;
cv::Mat img;
cv::Mat invertedPerspectiveMatrix;
    
void image_processing(cv::Mat orig_image){
    
	if(first_time){
    
		// Inicio para imágenes 640x480, x1 -90 x2 550 x3 600 x4 -190 y1 250 y2 300
		x1 = 50;
		x2 = 630;
		x3 = 690;
		x4 = -70;
		y_1 = 190;
		y2 = 260;
     
		kfd = 0;
		sign = 1;
		dstVertices[0] = cv::Point(0, 0);
		dstVertices[1] = cv::Point(639, 0);
		dstVertices[2] = cv::Point(639, 281);
		dstVertices[3] = cv::Point(0, 281);
		tcgetattr(kfd, &cooked);
		memcpy(&raw, &cooked, sizeof(struct termios));
		raw.c_lflag &=~ (ICANON | ECHO);
		raw.c_cc[VEOL] = 1;
		raw.c_cc[VEOF] = 2;
		tcsetattr(kfd, TCSANOW, &raw);
		
		puts("Reading from keyboard");
		puts("---------------------------");
		puts("Moving around:");
		puts("   i q w e r    ");
		puts("---------------------------");
		
		orig = orig_image.clone();

		first_time=false;

	}
	
	while(!die){
		// Funciona mejor todavía
		srcVertices[0] = cv::Point(x1, y_1);
		srcVertices[1] = cv::Point(x2, y_1);
		srcVertices[2] = cv::Point(x3, y2);
		srcVertices[3] = cv::Point(x4, y2);    
		// Prepare matrix for transform and get the warped image 
		cv::Mat perspectiveMatrix = getPerspectiveTransform(srcVertices, dstVertices);
		cv::Mat dst(281, 639, CV_8UC3);
		//Destination for warped 
		//For transforming back into original image space 
		cv::invert(perspectiveMatrix, invertedPerspectiveMatrix);
		cv::warpPerspective(orig, dst, perspectiveMatrix, dst.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);
		cv::imshow("Display Image Gray", dst);
		cv::waitKey(1);

		if(read(kfd, &c, 1) < 0){
			perror("read():");
			exit(-1);
		}
		switch(c){
		case KEYCODE_I:
			sign *= -1;
			break;
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
			y_1 = y_1 + sign * 10;
			break;
		case KEYCODE_PERIOD:
			y2 = y2 + sign * 10;
			break;
		}
		std::cout << "x1 " << x1;
		std::cout << " x2 " << x2;
		std::cout << " x3 " << x3;
		std::cout << " x4 " << x4;
		std::cout << " y1 " << y_1;
		std::cout << " y2 " << y2;
		std::cout << " sign " << sign << std::endl;
		std::cout.flush();    
	}   
}
    

int main(int argc, char** argv )
{
	if ( argc != 2 ){
		printf("usage: DisplayImage.out <Image_Path>\n");
		return -1;
	}
	
	orig_image = cv::imread( argv[1], 1 );
	
	if ( !orig_image.data ){
		printf("No image data \n");
		return -1;
	}
	
	image_processing(orig_image);
	exit(-1);
	
	return 0;
}
