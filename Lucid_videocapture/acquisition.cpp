#include "acquisition.h"

using cv::Mat;
using cv::Point;
using cv::Scalar;

using namespace std;

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

//若连续采集，则无中断
//若采集视频，则按任意键中断

int getVideo(Arena::IDevice* pDevice, int mode)
{
    /*******************Load the Video infomation from config.yaml **********************/

    // Open the YAML file
  	std::ifstream fin("camera_config.yaml");
  	YAML::Node config = YAML::Load(fin);

  	// Read the values from the YAML file
  	string VideoPath = config["VideoPath"].as<string>();

    // Read timeout of img acquisition
    double TimeOut = config["TimeOut"].as<double>();

    // fps for Acquisition and VideoWriter
    double fps = config["fps"].as<double>();

    // ExposureTime and Gain for print
    string str_ExposureTime = to_string(config["ExposureTime"].as<double>()/1000);
    string str_Gain = to_string(config["Gain"].as<double>());

    // Video Length
    double VideoLength = config["VideoLength"].as<double>();

    // Video coding
    string VideoCoding = config["VideoCoding"].as<string>();

    // AcquisitionFrameRate
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(), "AcquisitionFrameRate", fps);

    // Image size
    int imgWidth = int(config["Width"].as<double>());
    int imgHeight = int(config["Height"].as<double>());

	// Close the YAML file
	fin.close();

	//定义图像采集指针
	Arena::IImage* pImage;

	// gamma
	double gamma = 0.5;

	// 8bit gamma LUT
	Mat lookUpTable_8bit(1, 256, CV_8U);
	create8bitGammaTable(lookUpTable_8bit, gamma);

	// Video object
	cv::VideoWriter vw;
	
    // File name
    string filename = createFileName();

	cout << "Save the video at: " + VideoPath + filename +".avi" << endl;

	//创建录像文件
	vw.open(  VideoPath + filename +".avi", //路径
        cv::VideoWriter::fourcc(VideoCoding[0], VideoCoding[1], 
        VideoCoding[2], VideoCoding[3]), 
		//VideoWriter::fourcc('M', 'J', 'P', 'G'), //压缩率低
		//VideoWriter::fourcc('D', 'I', 'V', 'X'), //压缩率高
		//VideoWriter::fourcc('D', 'I', 'V', '3'), //压缩率中等
		//VideoWriter::fourcc('M', 'P', '4', '2'), //压缩率中等
		fps, //帧率
		cv::Size(imgWidth,imgHeight),  //尺寸
		true
	);

	//确认录像功能开启成功
	if (!vw.isOpened())
	{
		cout << "Unable to initialize recording！" << endl;
		getchar();
		return -1; // return -1 for video capture failed
	}
	cout << "Recording started!" << endl;

	//相机开始采集流
	pDevice->StartStream();

	// Time string to print in image
	string time_string;

	// Exposure info string to print in image
	string exposure_string;

	// 创建BayerRG图像暂存容器
	Mat src;

	// 创建彩色图暂存容器
	Mat color_img = cv::Mat(src.rows, src.cols, CV_8UC3, cv::Scalar(0, 0, 0));

	// 用于高精度计时，截止视频
	high_resolution_clock::time_point start_time =  high_resolution_clock::now();
	
	// 用于中断采集
	int interrupt = 0;
	// Note for interrupt: 
	// 0 for normal acquisition, 1 for interruption by user, -1 for videocapture failed

	// 不间断采集
	while(true)
	{
		//图像采集指针从buffer获取图像
		pImage = pDevice->GetImage(TimeOut);

		//将Arena图像指针指向的图像数据转移到OpenCV的Mat类对象，格式为每次取1字节
		src = cv::Mat((int)pImage->GetHeight(), (int)pImage->GetWidth(), CV_8UC1, (void *)pImage->GetData());
		
        // Gamma correction
		gammaCorrection_8bit_original(lookUpTable_8bit, src, color_img);
		
		//以采集图像时刻制作时间戳
		time_string = createTimeString();

		//制作曝光信息字符串,曝光时间和增益分别保留6位和3位有效数字
		exposure_string = "E:" + str_ExposureTime.substr(0,7)
			+ " G:" + str_Gain.substr(0,4);

		/*****************黑白双色时间戳*******************/

		//采集的图像中加入时间戳
		cv::putText(color_img, time_string,
			Point(5, 15),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, // font face and scale
			Scalar(255, 255, 255), // 白色字体
			1, cv::LINE_AA); // line thickness and type
		cv::putText(color_img, time_string,
			Point(5, 30),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, // font face and scale
			Scalar(0, 0, 0), // 黑色字体
			1, cv::LINE_AA); // line thickness and type

		//采集的图像中加入曝光信息
		cv::putText(color_img, exposure_string,
			Point(5, 45),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, // font face and scale
			Scalar(255, 255, 255), // 白色字体
			1, cv::LINE_AA); // line thickness and type
		cv::putText(color_img, exposure_string,
			Point(5, 60),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, // font face and scale
			Scalar(0, 0, 0), // 黑色字体
			1, cv::LINE_AA); // line thickness and type

		// Show current image
		cv::imshow("Lucid Video", color_img);
		
		//将图像写入录像文件
		vw.write(color_img);

		//释放Buffer
		pDevice->RequeueBuffer(pImage);

		// Get current time
		high_resolution_clock::time_point end_time =  high_resolution_clock::now();
		
        // Calculate time duration of video capture UNTIL NOW
		milliseconds duration_time = std::chrono::duration_cast<milliseconds>(end_time- start_time);
		
        // Stop video capture if time is out , only when continuous mode is used
        if( duration_time.count()/1000  >  60* VideoLength && mode == 1) //VideoLength
		    break;

		//当有按键按下时，退出循环
		if (cv::waitKey(1) >= 0)
		{
			//连续采集模式
			if(mode == 1)
				interrupt = 1;
			break;
		}
		
	}

	// Stop stream
	pDevice->StopStream();

	// Release VideoWriter
	vw.release();

	return interrupt;
}

// 采集标定图像
int getCalibrationImages(Arena::IDevice* pDevice)
{
	// let the user input the number of images to be captured
	int numImages = 0;
	cout << "Enter the number of images to be captured: ";
	cin >> numImages;

	if(numImages <= 0)
	{
		cout << "Invalid number of images!" << endl;
		return -1;
	}

	// the number of images that already have been captured
	int imagesCaptured = 0;

	/*******************Load the Video infomation from config.yaml **********************/

    // Open the YAML file
  	std::ifstream fin("camera_config.yaml");
  	YAML::Node config = YAML::Load(fin);

  	// Read the values from the YAML file
  	string VideoPath = config["VideoPath"].as<string>();

    // Read timeout of img acquisition
    double TimeOut = config["TimeOut"].as<double>();

    // ExposureTime and Gain for print
    string str_ExposureTime = to_string(config["ExposureTime"].as<double>()/1000);
    string str_Gain = to_string(config["Gain"].as<double>());

    // Image size
    int imgWidth = int(config["Width"].as<double>());
    int imgHeight = int(config["Height"].as<double>());

	// Close the YAML file
	fin.close();

	//定义图像采集指针
	Arena::IImage* pImage;

	// gamma
	double gamma = 0.5;

	// 8bit gamma LUT
	Mat lookUpTable_8bit(1, 256, CV_8U);
	create8bitGammaTable(lookUpTable_8bit, gamma);

	//相机开始采集流
	pDevice->StartStream();

	// Time string to print in image
	string time_string;

	// Exposure info string to print in image
	string exposure_string;

	// 创建BayerRG图像暂存容器
	Mat src;

	// 创建彩色图暂存容器
	Mat color_img = cv::Mat(src.rows, src.cols, CV_8UC3, cv::Scalar(0, 0, 0));

	// folder for calibration images
	string folderName = "calibration_img/";

	// 不间断采集
	while(true)
	{
		//图像采集指针从buffer获取图像
		pImage = pDevice->GetImage(TimeOut);

		//将Arena图像指针指向的图像数据转移到OpenCV的Mat类对象，格式为每次取1字节
		src = cv::Mat((int)pImage->GetHeight(), (int)pImage->GetWidth(), CV_8UC1, (void *)pImage->GetData());
		
        // Gamma correction
		gammaCorrection_8bit_original(lookUpTable_8bit, src, color_img);

		// copy of color_img
		Mat img = color_img.clone();

		//采集的图像中加入图像序号
		cv::putText(color_img, "Get the " + to_string(imagesCaptured + 1) 
			+ "th image for calibration",
			Point(10, 30),
			cv::FONT_HERSHEY_SIMPLEX, 0.8, // font face and scale
			Scalar(0, 0, 255), // 红色字体
			1.5, cv::LINE_AA); // line thickness and type

		// Show current image
		cv::imshow("Lucid Calibration", color_img);

		//释放Buffer
		pDevice->RequeueBuffer(pImage);

		//当有按键按下时，保存图像
		if (cv::waitKey(20) >= 0)
		{
			imagesCaptured++;
			cv::imwrite(VideoPath + folderName + to_string(imagesCaptured) + ".png",img);
		}

		// 达到预计采集图像数量时，退出循环
		if(imagesCaptured >= numImages)
			break;
			
	}

	// Stop stream
	pDevice->StopStream();

	return imagesCaptured;
}