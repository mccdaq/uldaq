/*
 * UsbDio32hs.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "UsbDio32hs.h"
#include "./dio/DioUsbDio32hs.h"

namespace ul
{
UsbDio32hs::UsbDio32hs(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName) : UsbFpgaDevice(daqDeviceDescriptor, fpgaFileName)
{
	setCmdValue(CMD_STATUS_KEY, 0x40);
	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(96000000);

	setDioDevice(new DioUsbDio32hs(*this));

	setOverrunBitMask(0x0004);
	setUnderrunBitMask(0x0010);
	setScanRunningBitMask(SD_INPUT, 0x0002);
	setScanRunningBitMask(SD_OUTPUT, 0x0008);
	setScanDoneBitMask(0x40);

	mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_OUTPUT_SCAN | DE_ON_OUTPUT_SCAN_ERROR);

	setMultiCmdMem(false);
	setMemUnlockAddr(0x8000);
	setMemUnlockCode(0xAA55);

	addMemRegion(MR_USER, 0x7000, 4096, MA_READ | MA_WRITE);
}

UsbDio32hs::~UsbDio32hs()
{

}

unsigned char UsbDio32hs::getTrigModeCode(TriggerType type) const
{
	unsigned char code;

	unsigned char LEVEL = 0;
	unsigned char EDGE = 1;
	unsigned char LOW = 0;
	unsigned char HIGH = 1;
	unsigned char NEG_POL = 0;
	unsigned char POS_POL = 1;

	switch (type)
	{
		case TRIG_POS_EDGE:
			code = EDGE | POS_POL << 1;
			break;
		case TRIG_NEG_EDGE:
			code = EDGE | NEG_POL << 1;
			break;
		case TRIG_HIGH:
			code = LEVEL | HIGH << 1;
			break;
		case TRIG_LOW:
			code = LEVEL | LOW << 1;
			break;
		default:
			throw UlException(ERR_BAD_TRIG_TYPE);
	}

	return code;
}

void UsbDio32hs::setupTrigger(FunctionType functionType, ScanOption options) const
{
	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		TriggerConfig trigCfg = getTriggerConfig(functionType);

		//mRetrigCount = trigCfg.retrigCount;

		if(trigCfg.type & (TRIG_POS_EDGE | TRIG_NEG_EDGE | TRIG_HIGH | TRIG_LOW))
		{
			#pragma pack(1)
				struct
				{
					struct
					{
						unsigned char mode      : 1;
						unsigned char polarity  : 1; // 0 = low / falling, 1 = high / rising
						unsigned char resv		: 6;
					};
				} trigSettings;
			#pragma pack()

			switch(trigCfg.type)
			{
			case TRIG_HIGH:
				trigSettings.mode = LEVEL;
				trigSettings.polarity = HIGH;
				break;
			case TRIG_LOW:
				trigSettings.mode = LEVEL;
				trigSettings.polarity = LOW;
				break;
			case TRIG_POS_EDGE:
				trigSettings.mode = EDGE;
				trigSettings.polarity = POS_POL;
				break;
			case TRIG_NEG_EDGE:
				trigSettings.mode = EDGE;
				trigSettings.polarity = NEG_POL;
				break;
			default:
				break;
			}

			sendCmd(CMD_TRIG_CONFIG, 0, 0, (unsigned char*) &trigSettings, sizeof(trigSettings));

			//mPatternTrig = false;

		}
		else
		{
			#pragma pack(1)
				struct
				{
					unsigned short value;
					unsigned short mask;

					struct
					{
						unsigned char port			: 1;
						unsigned char type		 	: 2;
						unsigned char reserved1  	: 5;
					} options;

				} patternTrigSetting;
			#pragma pack ()

			unsigned short trigTypeCode = 0;

			switch(trigCfg.type)
			{
			case TRIG_PATTERN_EQ:
				trigTypeCode = 0;
				break;
			case TRIG_PATTERN_NE:
				trigTypeCode = 1;
				break;
			case TRIG_PATTERN_ABOVE:
				trigTypeCode = 2;
				break;
			case TRIG_PATTERN_BELOW:
				trigTypeCode = 3;
				break;
			default:
				break;
			}

			patternTrigSetting.value = trigCfg.level;
			patternTrigSetting.mask = trigCfg.variance;
			patternTrigSetting.options.type = trigTypeCode;

			patternTrigSetting.options.port = 0;
			if(trigCfg.trigChan == AUXPORT1)
				patternTrigSetting.options.port = 1;

			patternTrigSetting.options.reserved1 = 0;

			sendCmd(CMD_PATTERN_TRIG_CONFIG, 0, 0, (unsigned char*) &patternTrigSetting, sizeof(patternTrigSetting));

			//mPatternTrig = true;
		}

	}
}

} /* namespace ul */
