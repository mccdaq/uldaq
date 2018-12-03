/*
 * Usb1808.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "Usb1808.h"
#include "./daqi/DaqIUsb1808.h"
#include "./daqo/DaqOUsb1808.h"
#include "./ai/AiUsb1808.h"
#include "./ao/AoUsb1808.h"
#include "./dio/DioUsb1808.h"
#include "./ctr/CtrUsb1808.h"

namespace ul
{

Usb1808::Usb1808(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName) : UsbFpgaDevice(daqDeviceDescriptor, fpgaFileName)
{
	setCmdValue(CMD_STATUS_KEY, 0x40);
	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(100000000);

	setDaqIDevice(new DaqIUsb1808(*this));
	setDaqODevice(new DaqOUsb1808(*this));

	setAiDevice(new AiUsb1808(*this));
	setAoDevice(new AoUsb1808(*this, 2));
	setDioDevice(new DioUsb1808(*this));
	setCtrDevice(new CtrUsb1808(*this, 4));
	setTmrDevice(new TmrUsb1808(*this, 2));

	setOverrunBitMask(0x0004);
	setUnderrunBitMask(0x0010);
	setScanRunningBitMask(SD_INPUT, 0x0002);
	setScanRunningBitMask(SD_OUTPUT, 0x0008);
	setScanDoneBitMask(0x40);

	mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_OUTPUT_SCAN | DE_ON_OUTPUT_SCAN_ERROR);

	setMultiCmdMem(false);
	setMemUnlockAddr(0x8000);
	setMemUnlockCode(0xAA55);

	addMemRegion(MR_CAL, 0x7000, 278, MA_READ);
	addMemRegion(MR_USER, 0x7200, 3584, MA_READ | MA_WRITE);
}

Usb1808::~Usb1808()
{
}

void Usb1808::setupTrigger(FunctionType functionType, ScanOption options) const
{
	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		TriggerConfig trigCfg = getTriggerConfig(functionType);

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

		}
		else
		{
			#pragma pack(1)
				struct
				{
					unsigned char value;
					unsigned char mask;

					struct
					{
						unsigned char reserved0  : 1;
						unsigned char type		 : 2;
						unsigned char reserved1  : 5;
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

			patternTrigSetting.options.reserved0 = 0;
			patternTrigSetting.options.reserved1 = 0;

			sendCmd(CMD_PATTERN_TRIG_CONFIG, 0, 0, (unsigned char*) &patternTrigSetting, sizeof(patternTrigSetting));
		}

	}
}

} /* namespace ul */
