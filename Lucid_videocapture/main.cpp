/***************************************************************************************
								 Lucid相机采集图像
 ***************************************************************************************/
// 
#include "lucid_videocapture.h"

// self-defined headers
#include "camera_initialization.h"
#include "acquisition.h"
#include "time_stamp.h"

/*************************************宏定义******************************************/

// //等待相机连接启动
// #define TIME_FOR_BOOT 1000

// //图像采集等待时间
// #define TIMEOUT 2000

// //定义曝光时间最大值(单位us)
// #define EXPOSURE_MAX 10000 
// //25fps时，每帧有40ms的时间，去掉其他算法的时间，留给曝光时间应小于30ms

// //定义每个短视频的时长，单位minute
// #define TIME_OF_VIDEO 5

// const enum for capture mode
enum CaptureMode
{
	CONTINUOUS = 1,
	VIDEO = 2,
	CALIBRATION = 3
};

/*************************************主函数******************************************/

int main()
{
    // Open log file
	std::ofstream log;
	log.open("log.txt", std::ios_base::app); // append instead of overwrite
	string last_time_start = createTimeString();
	log << std::endl << "Last time start: " <<  last_time_start << "   ";

	bool exceptionThrown = false;

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
			log << "No camera connected\n";
			std::getchar();
			return 0;
		}

		log << "Camera connected\n";

		//若设备存在，获取第一台设备的设备指针
		Arena::IDevice* pDevice = pSystem->CreateDevice(deviceInfos[0]);

		

        //初始化设备
        cameraInitialization(pDevice);


		//运行采集图像的函数
		
		
		//请使用者选择采集模式，若选择“1”，则连续采集，若选择“2”，则采集一段时间的视频，若选择“3”，则采集若干张图片用于标定
		int mode;
		std::cout << "Please choose the mode of acquisition: \n";
		std::cout << "1. Continuous acquisition\n";
		std::cout << "2. Acquisition of a video\n";
		std::cout << "3. Acquisition of several images for calibration\n";
		std::cin >> mode;

		//若选择“CONTINUOUS_VIDEO”，则连续采集
		if(mode == CaptureMode::CONTINUOUS)
		{
			int numofVideos = 0;
			while(true)
			{
				// 若有按键输入，则停止采集，否则一直采集
				int ret = getVideo(pDevice, mode);
				if(ret == 1)
					break;
				if(ret == 0)
				{
					numofVideos++;
					std::cout << "Video " << numofVideos << " is captured.\n";
				}	
			}
		}

		//若选择“ONE_VIDEO”，则采集一段视频
		else if(mode == CaptureMode::VIDEO)
		{
			getVideo(pDevice, mode);
			std::cout << "The video is captured.\n";
		}

		//若选择“CALIBRATION_IMAGES”，则采集若干张图片用于标定
		else if(mode == CaptureMode::CALIBRATION)
		{
			int ret = getCalibrationImages(pDevice);
			if(ret == -1)
				std::cout << "Invalid number of calibration images input by user.\n";
			else
				std::cout << ret << 
			" images are captured for calibration.\n";
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

    log.close();

	if (exceptionThrown)
		return -1;
	else
		return 0;
}