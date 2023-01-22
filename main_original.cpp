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
#include <atomic>

#include <signal.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <opencv2/opencv.hpp>

#include "image_processing.hpp"
#include "glwindow.hpp"

// Global
volatile bool die = false;

// -----------------------------------------------
static void show_usage(std::string name){
    std::cerr << "Usage: " << name << " <option(s)>"
              << "Options:\n"
              << "\t_help:=true\t\tShow this help message\n"
              << "\t_display_windows:=true\t\tDisplay windows\n"
              << "\t-w,--width\t\tImage width\n"
              << "\t-e,--height\t\tImage height\n"
              << std::endl;
}

// -----------------------------------------------
void my_signal_handler(int /*signum*/)
{
  // ok, lock-free atomics
  die = true;

  const char str[] = "\nStopping program\n";
  // ok, write is signal-safe
  write(STDERR_FILENO, str, sizeof(str) - 1); 

}

int main(int argc, char **argv){

 int res;
 pthread_t image_capture_thread, image_processing_thread;

 // Command-line parameters
 bool display_windows = true; 
 bool help = false; 

    if (display_windows == true){
       std::cout << "display_windows = " << display_windows << std::endl; 
    }
 
  // setup signal handler
    struct sigaction action;
    action.sa_handler = my_signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
 
    if (argc > 7) {
        exit(EXIT_FAILURE);
    }
             
    // HHAA This line MUST be here    
    if (!glfw_Init(display_windows))
         exit(EXIT_FAILURE);
    std::cout << "GLFW initiated succesfully" << std::endl;       
    glfwSetWidthHeight(640, 480);   
               
    res = pthread_create(&image_capture_thread, NULL, image_capture_threadfunc, NULL); //(void *) argv[1]);
    if (res){
       printf("Image capture pthread_create failed\n");
       return 1;
    } 
    res = pthread_create(&image_processing_thread, NULL, image_processing_threadfunc, NULL);
    if (res){
       printf("Image processing pthread_create failed\n");
       return 1;
    }


    // with pthread_ join the main function will wait for the thread(s) to terminate
    pthread_join(image_capture_thread, NULL);       
    pthread_join(image_processing_thread, NULL);    

    // HHAA These lines MUST be here
    glfwDestroyWindows();    
       
    exit(EXIT_SUCCESS);

}
