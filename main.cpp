#include <stdio.h>
#include <opencv2/opencv.hpp>

int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    cv::Mat original;
    original = cv::imread( argv[1], 1 );
    if ( !original.data )
    {
        printf("No image data \n");
        return -1;
    }

    cv::imshow("Display Image Original", original);
    cv::waitKey(0);
    return 0;
    }
