#ifndef ACQUISITION_H
#define ACQUISITION_H
#include "lucid_videocapture.h"
#include "gammaLUT.h"
#include "time_stamp.h"

// the return value shows if the acquisition is interrupted by user
int getVideo(Arena::IDevice* pDevice, int mode);

// the return value shows the number of images captured
int getCalibrationImages(Arena::IDevice* pDevice);


#endif ACQUISITION_H