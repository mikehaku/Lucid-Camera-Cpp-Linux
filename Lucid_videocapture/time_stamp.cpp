#include "time_stamp.h"

using namespace std;

string createFileName(void)
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

    return filename;
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
	str = to_string(1900+p->tm_year) + "." + Month + "." + Day 
		+ " " + Hour + ":" + Minite + ":" + Second + "." + Mili;

	//高精度时间
	//high_resolution_clock::time_point start_time =  high_resolution_clock::now();


	//返回时间戳字符串
	return str;
}

    