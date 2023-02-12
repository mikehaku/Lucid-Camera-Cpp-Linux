#include "gammaLUT.h"

//创建8bit伽马校正查找表
void create8bitGammaTable(Mat& lookUpTable, double gamma)
{
	//创建查找表指针
	uchar* p = lookUpTable.ptr();

	//写入查找值
	for (int i = 0; i < 256; ++i)
		p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
}

//执行8bit伽马校正
void gammaCorrection_8bit_original(Mat& lookUpTable, Mat& src, Mat& dst)
{
	//创建暂存Mat对象
	Mat img_gamma;

	//执行伽马校正
	cv::LUT(src, lookUpTable, img_gamma);

	//从BayerRG8转换到BGR彩色图
	cv::cvtColor(img_gamma, dst, cv::COLOR_BayerRG2RGB);

	//释放Mat对象
	img_gamma.release();
}