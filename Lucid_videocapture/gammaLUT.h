#ifndef GAMMALUT_H
#define GAMMALUT_H
#include "lucid_videocapture.h"

using cv::Mat;

//创建8bit伽马校正查找表
//@输入：查找表Mat对象、伽马值
//@输出：无
void create8bitGammaTable(Mat& lookUpTable, double gamma);

//执行8bit伽马校正
//@输入：查找表Mat对象、输入Mat对象(BayerRG8)、输出Mat对象(RGB8)
//@输出：无
void gammaCorrection_8bit_original(Mat& lookUpTable, Mat& src, Mat& dst);



#endif