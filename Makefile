CC = g++
CFLAGS =  `pkg-config --cflags opencv4` 
LFLAGS =  `pkg-config --libs opencv4` -lglfw -lglut -lGLU -lGL -lGLEW -lpthread -lm  -lopencv_videoio -lopencv_imgcodecs -lopencv_core -lopencv_imgproc -lopencv_features2d -lopencv_flann -lopencv_video -lopencv_calib3d -lopencv_highgui 
# https://stackoverflow.com/questions/14492436/g-optimization-beyond-o3-ofast
OPTIONS = -g #-Ofast -march=native -flto

PROGRAM = image_transform 
OBJECTS =  main.o 

all :  $(PROGRAM)
	 
image_transform : $(OBJECTS)
	 $(CC) $(MATH_F) $(OBJECTS) -o image_transform  $(LFLAGS) $(OPTIONS)

main.o:  main.cpp 
	$(CC)  main.cpp -c  $(OPTIONS) $(CFLAGS) -o  main.o

clean:
	rm -f core core.* *.*~ *.o lane_detection *~ 


