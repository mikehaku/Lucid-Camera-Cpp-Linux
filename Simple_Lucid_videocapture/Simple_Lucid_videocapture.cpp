/***************************************************************************************
								 Lucid相机采集图像
 ***************************************************************************************/

/*************************************包含文件******************************************/

//包含常用C++库
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <chrono>

//包含相机需要的库文件
#include "stdafx.h"
#include "ArenaApi.h"

//包含常用的OpenCV库
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

//包含读取配置的库
#include <fstream>

//系统时间库
//#include <windows.h>
	//注意，该时间库为windows的系统时间库。
	//Linux系统下应改为对应的系统时间库，
	//并对“创建时间戳字符串”函数做对应修改。

//Linux系统时间库
#include <sys/time.h>

/*************************************宏定义******************************************/

//等待相机连接启动
#define TIME_FOR_BOOT 1000

//图像采集等待时间
#define TIMEOUT 2000

//定义曝光时间最大值(单位us)
#define EXPOSURE_MAX 10000 
//25fps时，每帧有40ms的时间，去掉其他算法的时间，留给曝光时间应小于30ms

//定义每个短视频的时长，单位minute
#define TIME_OF_VIDEO 5

//定义白平衡基数，可以直接修改参数


/*************************************命名空间******************************************/

using namespace std;
using namespace cv;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;


/*************************************函数声明******************************************/

//相机参数初始化
//@输入：设备指针
void CamParaInit(Arena::IDevice* pDevice, vector<double> _cameraPara);

//创建8bit伽马校正查找表
//@输入：查找表Mat对象、伽马值
//@输出：无
void create8bitGammaTable(Mat& lookUpTable, double gamma);

//执行8bit伽马校正
//@输入：查找表Mat对象、输入Mat对象(BayerRG8)、输出Mat对象(RGB8)
//@输出：无
void gammaCorrection_8bit_original(Mat& lookUpTable, Mat& src, Mat& dst);

//创建时间戳字符串
//@输入：系统时间
//@输出：时间戳字符串
//string createTimeString(SYSTEMTIME& sys);

//创建时间戳字符串(Linux)
//@输入：系统时间
//@输出：时间戳字符串
string createTimeString(void);

//图像采集函数
//@输入：设备指针IDevice
//@输出：无
void AcquireImages(Arena::IDevice* pDevice, vector<double> _cameraPara);


/*************************************函数定义******************************************/

//相机参数初始化
void CamParaInit(Arena::IDevice* pDevice, vector<double> _cameraPara)
{
	//获取相机参数向量
	double exposureTime = _cameraPara[0];
	double Gain = _cameraPara[1];

	//获得当前采集模式
	GenICam::gcstring acquisitionModeInitial = Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "AcquisitionMode");

	//将采集模式设置为“连续模式”
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"AcquisitionMode",
		"Continuous");

	//关闭自动增益
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"GainAuto",
		"Off");

	//将初始状态增益设为0
	GenApi::CFloatPtr pGain = pDevice->GetNodeMap()->GetNode("Gain");
	pGain->SetValue(Gain);

	//关闭自动曝光
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"ExposureAuto",
		"Off");

	//获取当前曝光时间
	GenApi::CFloatPtr pExposureTime = pDevice->GetNodeMap()->GetNode("ExposureTime");

	//设置初始曝光时间
	pExposureTime->SetValue(exposureTime);

	//设置buffer处理模式为“仅采集最新图像”
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetTLStreamNodeMap(),
		"StreamBufferHandlingMode",
		"NewestOnly");

	//允许自动调节数据包的尺寸
	Arena::SetNodeValue<bool>(
		pDevice->GetTLStreamNodeMap(),
		"StreamAutoNegotiatePacketSize",
		true);

	//允许数据包重发
	Arena::SetNodeValue<bool>(
		pDevice->GetTLStreamNodeMap(),
		"StreamPacketResendEnable",
		true);

	//设置感光参数，红绿蓝按比例
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(),
		"BalanceRatioSelector", "Red");
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(),
		"BalanceRatio", 1.76);
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(),
		"BalanceRatioSelector", "Green");
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(),
		"BalanceRatio", 1.0);
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(),
		"BalanceRatioSelector", "Blue");
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(),
		"BalanceRatio", 1.58);
}

//创建8bit伽马校正查找表
void create8bitGammaTable(Mat& lookUpTable, double gamma)
{
	//创建查找表指针
	uchar* p = lookUpTable.ptr();

	//写入查找值
	for (int i = 0; i < 256; ++i)
		p[i] = saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
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

string createTimeString(void)
{
	//创建时间戳字符串
	string str;

	//获取Linux系统时间
	struct timeval    tv; 
    struct timezone tz; 
    struct tm   *p; 
    gettimeofday(&tv, &tz); 
	p = localtime(&tv.tv_sec); 

    	//提取日期和时间信息
	string Month = to_string(1+p->tm_mon);
	string Day = to_string(p->tm_mday);
	string Hour = to_string(p->tm_hour);
	string Minite = to_string(p->tm_min);
	string Second = to_string(p->tm_sec);
	string Mili = to_string(tv.tv_usec);

	//补齐位数
	Month = Month.length() > 1 ? Month : ("0" + Month);
	Day = Day.length() > 1 ? Day : ("0" + Day);
	Hour = Hour.length() > 1 ? Hour : ("0" + Hour);
	Minite = Minite.length() > 1 ? Minite : ("0" + Minite);
	Second = Second.length() > 1 ? Second : ("0" + Second);
	switch (Mili.length())
	{
	case 1:Mili = "00" + Mili;
		break;
	case 2:Mili = "0" + Mili;
	default:
		break;
	}

	//连接时间戳字符串
	str = to_string(1900+p->tm_year) + "." + Month + "." + Day + " " + Hour + ":" + Minite + ":" + Second + "." + Mili;

	//返回时间戳字符串
	return str;
}


//图像采集函数
void AcquireImages(Arena::IDevice* pDevice, vector<double> _cameraPara)
{
	//获取Linux系统时间
	struct timeval    tv; 
    struct timezone tz; 
    struct tm   *p; 
    gettimeofday(&tv, &tz); 
	p = localtime(&tv.tv_sec); 

    //提取日期和时间信息
	string Month = to_string(1+p->tm_mon);
	string Day = to_string(p->tm_mday);
	string Hour = to_string(p->tm_hour);
	string Minite = to_string(p->tm_min);
	string Second = to_string(p->tm_sec);

	//补齐位数
	Month = Month.length() > 1 ? Month : ("0" + Month);
	Day = Day.length() > 1 ? Day : ("0" + Day);
	Hour = Hour.length() > 1 ? Hour : ("0" + Hour);
	Minite = Minite.length() > 1 ? Minite : ("0" + Minite);
	Second = Second.length() > 1 ? Second : ("0" + Second);

	string filename = to_string(1900+p->tm_year)  + Month +  Day + "-" + Hour + Minite  + Second;

	//录像的储存路径，根据需要进行更改
	string path = "/home/haoyang/Arena/LucidVideo/";
	//string path = "";

	//相机参数初始化
	CamParaInit(pDevice, _cameraPara);

	//获取增益指针
	GenApi::CFloatPtr pGain = pDevice->GetNodeMap()->GetNode("Gain");

	//定义图像采集指针
	Arena::IImage* pImage;

	//设明亮环境下的伽马为0.55
	double gamma_bright = 0.55;

	//设黑暗环境下的伽马为0.5
	double gamma_dark = 0.5;
	
	//生成8bit伽马校正表(明亮)
	Mat lookUpTable_8bit(1, 256, CV_8U);
	create8bitGammaTable(lookUpTable_8bit, gamma_bright);

	//生成8bit伽马校正表(黑暗)
	Mat lookUpTable2_8bit(1, 256, CV_8U);
	create8bitGammaTable(lookUpTable2_8bit, gamma_dark);



	//设置帧率为整数24
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(), "AcquisitionFrameRate",24.0);



	//开启录像功能
	VideoWriter vw;

	cout << "here" << endl;
	//设置录像帧率
	double fps = 24.0;
	
	cout << path + filename +".avi" << endl;

	//创建录像文件
	vw.open(  path + filename +".avi", //路径
		//VideoWriter::fourcc('M', 'J', 'P', 'G'), //压缩率低
		VideoWriter::fourcc('D', 'I', 'V', 'X'), //压缩率高
		//VideoWriter::fourcc('D', 'I', 'V', '3'), //压缩率中等
		//VideoWriter::fourcc('M', 'P', '4', '2'), //压缩率中等
		fps, //帧率
		Size(1440,928),  //尺寸
		true
	);

	//确认录像功能开启成功
	if (!vw.isOpened())
	{
		cout << "Unable to initialize recording！" << endl;
		getchar();
		return;
	}
	cout << "Recording started!" << endl;

	//相机开始采集流
	pDevice->StartStream();

	//采集图像计数
	int count = 0;

	//时间字符串
	string time_string;

	//曝光信息字符串
	string exposure_string;

	//创建BayerRG图像暂存容器
	Mat img;

	//创建彩色图暂存容器
	Mat color_img = cv::Mat(img.rows, img.cols, CV_8UC3, Scalar(0, 255, 0));

	//用于退出图像采集程序的标志位
	bool out_flag = false;

//测试Linux系统时间功能
 createTimeString();

	//用于计时的变量
	clock_t start_c, end_c;
	start_c = clock();

	//用于高精度计时
	high_resolution_clock::time_point start_time =  high_resolution_clock::now();
	
	//不间断采集
	while(true)
	{

		//图像采集指针从buffer获取图像
		pImage = pDevice->GetImage(TIMEOUT);

		//获取系统时间，用于制作时间戳
		//GetLocalTime(&sys);

		//将Arena图像指针指向的图像数据转移到OpenCV的Mat类对象，格式为每次取1字节
		img = cv::Mat((int)pImage->GetHeight(), (int)pImage->GetWidth(), CV_8UC1, (void *)pImage->GetData());
		
		//判断是否处于黑暗环境：
		//黑暗条件下，牺牲色准换取更大的动态范围；
		//明亮条件下，牺牲一定的动态范围换取更好的色准。
		//黑暗的判断标准为：1.曝光时间达到极限 且 2.增益达到极限 

		//使用BayerRG8数据进行伽马校正
		// if (pGain->GetValue() == 42 && pExposureTime->GetValue() == EXPOSURE_MAX)
		// 	gammaCorrection_8bit_original(lookUpTable2_8bit, img, color_img);
		// else
			gammaCorrection_8bit_original(lookUpTable_8bit, img, color_img);
		
		//以采集图像时刻制作时间戳
		time_string = createTimeString();

		//制作曝光信息字符串,曝光时间和增益分别保留6位和3位有效数字
		exposure_string = "E:" + to_string(_cameraPara[0] / 1000).substr(0,7)
			+ " G:" + to_string(_cameraPara[1]).substr(0,4);


		//采用双色时间戳，增加场景适应能力
		//若遮挡画面，亦可取消
		//采集的图像中加入黑色的时间戳
		putText(color_img, time_string,
			Point(5, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, // font face and scale
			Scalar(255, 255, 255), // 白色字体
			1, LINE_AA); // line thickness and type

		//采集的图像中加入黑色的时间戳
		putText(color_img, time_string,
			Point(5, 30),
			FONT_HERSHEY_SIMPLEX, 0.5, // font face and scale
			Scalar(0, 0, 0), // 黑色字体
			1, LINE_AA); // line thickness and type


		//采集的图像中加入曝光信息
		putText(color_img, exposure_string,
			Point(5, 45),
			FONT_HERSHEY_SIMPLEX, 0.5, // font face and scale
			Scalar(255, 255, 255), // 白色字体
			1, LINE_AA); // line thickness and type

		//采集的图像中加入曝光信息
		putText(color_img, exposure_string,
			Point(5, 60),
			FONT_HERSHEY_SIMPLEX, 0.5, // font face and scale
			Scalar(0, 0, 0), // 白色字体
			1, LINE_AA); // line thickness and type
		
		//采集的图像中加入梯形边框
		Mat img_show;

		img_show = color_img;

		//在窗口中显示经过伽马处理的图像，若有按键则将退出flag置为true
		imshow("Lucid", img_show);
		//即使中间按键也不会出现视频中断
		waitKey(1) ;
		//	out_flag = true;
		
		//将图像写入录像文件
		vw.write(color_img);

		//释放Buffer
		pDevice->RequeueBuffer(pImage);

		//拍摄图像数量加1
		count++;

		//计时满某个长度后保存并开启下一个视频文件
		end_c = clock();
		high_resolution_clock::time_point end_time =  high_resolution_clock::now();
		//if(end_c - start_c > TIME_OF_VIDEO*60*CLOCKS_PER_SEC)
		milliseconds duration_time = std::chrono::duration_cast<milliseconds>(end_time- start_time);
		if( duration_time.count()/1000  >  30) //默认30秒
		out_flag = true;

		//若退出flag为真则退出采集
		if (out_flag)
			break;
	}

	//关闭相机图像流
	pDevice->StopStream();

	//释放录像功能
	vw.release();
}


/*************************************主函数******************************************/
int main()
{
	//测试读取文件能力
	ifstream config("config.txt");
	vector<double> cameraPara(2,0);
	config >> cameraPara[0];
	config >> cameraPara[1];

	//日志文件生成
	ofstream log;

	log.open("log.txt", std::ios_base::app); // append instead of overwrite
	string last_time_start = createTimeString();
	log << endl << "Last time start: " <<  last_time_start << "   ";

	//延时等待相机启动
	waitKey(TIME_FOR_BOOT);

	// flag to track when an exception has been thrown
	bool exceptionThrown = false;

	//显示“采集图像功能开启”
	std::cout << "Acquisition Started\n";

	try
	{
		//启动Arena系统
		Arena::ISystem* pSystem = Arena::OpenSystem();
		
		//更新系统
		pSystem->UpdateDevices(100);

		//获取设备信息
		std::vector<Arena::DeviceInfo> deviceInfos = pSystem->GetDevices();

		//若无设备信息，显示“相机未连接”，并退出
		if (deviceInfos.size() == 0)
		{
			std::cout << "\nNo camera connected\nPress ENTER to quit.\n";
			log << "No camera connected" << endl;
			std::getchar();
			return 0;
		}

		log << "Camera connected" << endl;

		//若设备存在，获取第一台设备的设备指针
		Arena::IDevice* pDevice = pSystem->CreateDevice(deviceInfos[0]);

		//运行采集图像的函数
		while(true)
		{
			AcquireImages(pDevice, cameraPara);
		}
		

		//报告程序结束
		std::cout << "\nAcquisition Ended.\n";

		//销毁设备
		pSystem->DestroyDevice(pDevice);

		//关闭系统
		Arena::CloseSystem(pSystem);
	}

	//以下为抛出各种异常
	catch (GenICam::GenericException& ge)
	{
		std::cout << "\nGenICam exception thrown: " << ge.what() << "\n";
		exceptionThrown = true;
		log <<  "\nGenICam exception thrown: " << ge.what() << "\n";
	}
	catch (std::exception& ex)
	{
		std::cout << "\nStandard exception thrown: " << ex.what() << "\n";
		exceptionThrown = true;
		log <<   "\nStandard exception thrown: " << ex.what() << "\n";
	}
	catch (...)
	{
		std::cout << "\nUnexpected exception thrown\n";
		exceptionThrown = true;
		log << "\nUnexpected exception thrown\n";
	}

	if (exceptionThrown)
		return -1;
	else
		return 0;
}
