#include "camera_initialization.h"

using std::string;

void cameraInitialization(Arena::IDevice* pDevice)
 {
	// Open the YAML file
  	std::ifstream fin("camera_config.yaml");
  	YAML::Node config = YAML::Load(fin);

	// Time before start
	cv::waitKey(
		config["TimeBeforeStart"].as<double>()
		);

	// flag to track when an exception has been thrown
	bool exceptionThrown = false;

	//显示“相机初始化”
	std::cout << "Camera initialization\n";

	//获得当前采集模式
	GenICam::gcstring acquisitionModeInitial = 
	Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "AcquisitionMode");

	//AcquisitionMode
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"AcquisitionMode",
		"Continuous");

	//GainAuto
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"GainAuto",
		"Off");

	//Gain
	GenApi::CFloatPtr pGain = pDevice->GetNodeMap()->GetNode("Gain");
	pGain->SetValue(
		config["Gain"].as<double>()
		);

	//ExposureAuto
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"ExposureAuto",
		"Off");

	//获取当前曝光时间
	GenApi::CFloatPtr pExposureTime = pDevice->GetNodeMap()->GetNode("ExposureTime");

	//ExposureTime
	pExposureTime->SetValue(
		config["ExposureTime"].as<double>()
		);

	//StreamBufferHandlingMode
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetTLStreamNodeMap(),
		"StreamBufferHandlingMode",
		"NewestOnly");

	//StreamAutoNegotiatePacketSize
	Arena::SetNodeValue<bool>(
		pDevice->GetTLStreamNodeMap(),
		"StreamAutoNegotiatePacketSize",
		"true");

	//StreamPacketResendEnable
	Arena::SetNodeValue<bool>(
		pDevice->GetTLStreamNodeMap(),
		"StreamPacketResendEnable",
		"true");

	//设置感光参数，红绿蓝按比例
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(),
		"BalanceRatioSelector", "Red");
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(),
		"BalanceRatio", config["BalanceRatioRed"].as<double>());
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(),
		"BalanceRatioSelector", "Green");
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(),
		"BalanceRatio", config["BalanceRatioGreen"].as<double>());
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(),
		"BalanceRatioSelector", "Blue");
	Arena::SetNodeValue<double>(pDevice->GetNodeMap(),
		"BalanceRatio", config["BalanceRatioBlue"].as<double>());

	//显示“相机初始化完成”
	std::cout << "Camera initialization completed.\n";

	// Close the YAML file
	fin.close();
 }
