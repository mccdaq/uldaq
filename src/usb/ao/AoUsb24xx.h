/*
 * AoUsb24xx.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_AO_AOUSB24XX_H_
#define USB_AO_AOUSB24XX_H_

#include "AoUsbBase.h"

namespace ul
{

class UL_LOCAL AoUsb24xx: public AoUsbBase
{
public:
	AoUsb24xx(const UsbDaqDevice& daqDevice, int numChans);
	virtual ~AoUsb24xx();

	virtual void initialize();

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);
	virtual void aOutArray(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[]);
	virtual double aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]);

	virtual UlError checkScanState(bool* scanDone) const;

protected:
	virtual void loadDacCoefficients();
	virtual int getCalCoefIndex(int channel, Range range) const  { return channel;}
	std::vector<CalCoef> getScanCalCoefs(int lowChan, int highChan, Range range, long long flags) const;
	void writeData_2408(int channel, int mode, AOutFlag flags, double dataValue);
	void writeData_2416(int channel, int mode, AOutFlag flags, double dataValue);

	short convertU16ToI16(unsigned short val);

	unsigned short setScanConfig(int lowChan, int highChan, unsigned int scanCount, double rate, ScanOption options, unsigned char* cfg);

	virtual unsigned int processScanData(void* transfer, unsigned int stageSize);

private:
	virtual unsigned int processScanData16_2416(libusb_transfer* transfer, unsigned int stageSize);

private:
	enum { CMD_AOUT = 0x18, CMD_AOUTSCAN_START = 0x19, CMD_AOUTSTOP = 0x1A, CMD_AOUTSCAN_STATUS = 0x1B};
	enum { FIFO_SIZE = 2 * 1024 };
	enum { NO_UPDATE = 0, UPDATE_CH = 1, UPDATE_ALL = 2 };

#pragma pack(1)
	struct
	{
		unsigned short pacer_period;
		unsigned short scan_count;
		unsigned char options;
	} mScanConfig_2416;

	struct
	{
		unsigned int pacer_period;
		unsigned short scan_count;
		unsigned char options;
	} mScanConfig_2408;
#pragma pack()


};

} /* namespace ul */

#endif /* USB_AO_AOUSB24XX_H_ */
