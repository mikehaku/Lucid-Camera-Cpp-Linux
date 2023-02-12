TARGET = Lucid_videocapture

ARCH_TYPE = $(shell dpkg --print-architecture)

ARENA_PATH = /home/haoyang/MyLibs/ArenaSDK_v0.1.54_Linux_x64/ArenaSDK_Linux_x64

ifeq ($(ARCH_TYPE), amd64)

LDFLAGS = -L$(ARENA_PATH)/lib64 \
          -L$(ARENA_PATH)/GenICam/library/lib/Linux64_x64 \
          -L$(ARENA_PATH)/ffmpeg \
          -L/usr/local/lib #directory of OpenCV lib 
          
GENICAMLIBS = -lGCBase_gcc54_v3_3_LUCID \
              -lGenApi_gcc54_v3_3_LUCID \
              -lLog_gcc54_v3_3_LUCID \
              -llog4cpp_gcc54_v3_3_LUCID \
              -lMathParser_gcc54_v3_3_LUCID \
              -lNodeMapData_gcc54_v3_3_LUCID \
              -lXmlParser_gcc54_v3_3_LUCID

OUTDIR = ./OutputDirectory/

else ifeq ($(ARCH_TYPE), armhf)

LDFLAGS = -L$(ARENA_PATH)/lib \
          -L$(ARENA_PATH)/GenICam/library/lib/Linux32_ARMhf \
          -L$(ARENA_PATH)/ffmpeg \
          -L/usr/local/lib #directory of OpenCV lib 

GENICAMLIBS = -lGCBase_gcc540_v3_3_LUCID \
              -lGenApi_gcc540_v3_3_LUCID \
              -lLog_gcc540_v3_3_LUCID \
              -llog4cpp_gcc540_v3_3_LUCID \
              -lMathParser_gcc540_v3_3_LUCID \
              -lNodeMapData_gcc540_v3_3_LUCID \
              -lXmlParser_gcc540_v3_3_LUCID


OUTDIR = ./OutputDirectory/

else ifeq ($(ARCH_TYPE), arm64)

LDFLAGS = -L$(ARENA_PATH)/lib \
          -L$(ARENA_PATH)/GenICam/library/lib/Linux64_ARM \
          -L$(ARENA_PATH)/ffmpeg\
          -L/usr/local/lib #directory of OpenCV lib 

GENICAMLIBS = -lGCBase_gcc54_v3_3_LUCID \
              -lGenApi_gcc54_v3_3_LUCID \
              -lLog_gcc54_v3_3_LUCID \
              -llog4cpp_gcc54_v3_3_LUCID \
              -lMathParser_gcc54_v3_3_LUCID \
              -lNodeMapData_gcc54_v3_3_LUCID \
              -lXmlParser_gcc54_v3_3_LUCID


OUTDIR = ./OutputDirectory/
endif

CC=g++



INCLUDE= -I$(ARENA_PATH)/include/Arena \
         -I$(ARENA_PATH)/include/Save \
         -I$(ARENA_PATH)/include/GenTL \
         -I$(ARENA_PATH)/GenICam/library/CPP/include \
		 -I/usr/local/include/opencv4 \
         -I/usr/local/include
         #directory of OpenCV include 
         

#add "-g" for debug mode
#-O2 makes balance for performance and efficiency
#-O3 has the best performance while sacrificing the efficiency
CFLAGS=-Wall -g -O2 -std=c++11 -Wno-unknown-pragmas


FFMPEGLIBS = -lavcodec \
             -lavformat \
             -lavutil \
             -lswresample

OPENCV_DIR=-L/usr/local/lib #directory of OpenCV lib 

OPENCV_LIBS= $(OPENCV_DIR) -lopencv_core \
			-lopencv_imgproc -lopencv_highgui \
			-lopencv_videoio -lopencv_imgcodecs 

YAML_DIR = -L/usr/local/lib #directory of yaml-cpp

YAML_LIBS = $(YAML_DIR) -lyaml-cpp

LIBS= -larena -lsave -lgentl $(GENICAMLIBS) $(FFMPEGLIBS) $(OPENCV_LIBS) $(YAML_LIBS) -lpthread -llucidlog 
RM = rm -f

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=%.o)
DEPS = $(OBJS:%.o=%.d)

.PHONY: all
all: ${TARGET}
	
${TARGET}: ${OBJS}
	${CC} ${LDFLAGS} $^ -o $@ $(LIBS)
	-mkdir -p $(OUTDIR)
	-cp $(TARGET) $(OUTDIR)

%.o: %.cpp ${SRCS}
	${CC} ${INCLUDE}  ${LDFLAGS} -o $@ $< -c ${CFLAGS}

${DEPS}: %.cpp
	${CC} ${CLAGS} ${INCLUDE} -MM $< >$@

-include $(OBJS:.o=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET} ${OBJS} ${DEPS}
