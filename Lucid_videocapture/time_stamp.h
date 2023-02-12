#ifndef TIME_STAMP_H
#define TIME_STAMP_H

#include "lucid_videocapture.h"

using std::string;

// Create file name with time stamp
string createFileName(void);

// Create time string to put in acquisited image
string createTimeString(void);

#endif