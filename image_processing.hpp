/*
# Released under MIT License

Copyright (c) 2022 Héctor Hugo Avilés Arriaga.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <iostream>
#include <chrono>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <opencv2/opencv.hpp>

void *image_processing_threadfunc(void *);
void *image_capture_threadfunc(void *);
bool open_rs2(void);
bool scan_pix_color(int x, int y, const cv::Mat &frame);
bool scan_pix_grey(int x, int y, const cv::Mat &frame);
bool scan_colorized_depth(int x, int y, const cv::Mat &frame);
void draw_blobs(std::vector <cv::Rect> &blobs, cv::Mat &frame);
int mssleep(long milliseconds);
std::vector<cv::Point2f> slidingWindow(cv::Mat image, cv::Rect window);
