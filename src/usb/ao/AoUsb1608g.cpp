/*
 * AoUsb1608g.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoUsb1608g.h"

namespace ul
{

AoUsb1608g::AoUsb1608g(const UsbDaqDevice& daqDevice, int numChans) : AoUsb1208hs(daqDevice, numChans)
{
	mAoInfo.setResolution(16);
	mAoInfo.setMaxScanRate(500000);
	mAoInfo.setMaxThroughput(500000 * numChans);
	mAoInfo.setFifoSize(FIFO_SIZE);

	mAoInfo.setCalCoefCount(numChans);
	mAoInfo.setCalCoefsStartAddr(0x7080);
	mAoInfo.setCalDateAddr(0x70A4);
}

AoUsb1608g::~AoUsb1608g()
{

}

void AoUsb1608g::readCalDate()
{
	unsigned short calDateBuf[6];
	int calDateAddr = mAoInfo.getCalDateAddr();

	if(calDateAddr != -1 && getScanState() == SS_IDLE)
	{
		int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, calDateAddr, (unsigned char*)calDateBuf, sizeof(calDateBuf));

		if(bytesReceived == sizeof(calDateBuf))
		{
			tm time;
			memset(&time, 0, sizeof(time));

			// the date is stored on the device in big-endian format by the test department
			time.tm_year =  mEndian.be_ui16_to_cpu(calDateBuf[0]) - 1900;
			time.tm_mon = mEndian.be_ui16_to_cpu(calDateBuf[1]) - 1;
			time.tm_mday = mEndian.be_ui16_to_cpu(calDateBuf[2]);
			time.tm_hour = mEndian.be_ui16_to_cpu(calDateBuf[3]);
			time.tm_min = mEndian.be_ui16_to_cpu(calDateBuf[4]);
			time.tm_sec = mEndian.be_ui16_to_cpu(calDateBuf[5]);
			time.tm_isdst = -1;

			time_t cal_date_sec = mktime(&time); // seconds since unix epoch

			if(cal_date_sec > 0) // mktime returns  -1 if cal date is invalid
				mCalDate = cal_date_sec;

			// convert seconds to string
			/*
			struct tm *timeinfo;
			timeinfo = localtime(&cal_date_sec);
			char b[100];
			strftime(b, 100, "%Y-%m-%d %H:%M:%S", timeinfo);
			std::cout << b << std::endl;*/
		}
	}
}

} /* namespace ul */
