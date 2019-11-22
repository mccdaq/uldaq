/*
 * Usb2020.cpp
 *
 *      Author: Measurement Computing Corporation
 */



#include "Usb2020.h"
#include "./ai/AiUsb2020.h"
#include "./dio/DioUsb2020.h"

namespace ul
{

Usb2020::Usb2020(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName) : UsbFpgaDevice(daqDeviceDescriptor, fpgaFileName)
{
	setCmdValue(CMD_STATUS_KEY, 0x40);
	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(80000000);

	setAiDevice(new AiUsb2020(*this));
	setDioDevice(new DioUsb2020(*this));

	setOverrunBitMask(0x0004);
	setScanRunningBitMask(SD_INPUT, 0x0002);

	mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR);

	setMultiCmdMem(false);
	setMemUnlockAddr(0x8000);
	setMemUnlockCode(0xAA55);

	addMemRegion(MR_CAL, 0x7000, 256, MA_READ);
	addMemRegion(MR_USER, 0x7140, 3768, MA_READ | MA_WRITE);
}

Usb2020::~Usb2020()
{

}

/*unsigned char Usb2020::getTrigModeCode(TriggerType type) const
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

void Usb2020::setupTrigger(FunctionType functionType, ScanOption options) const
{
	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		TriggerConfig trigCfg = getTriggerConfig(functionType);

		unsigned char code = getTrigModeCode(trigCfg.type);

		sendCmd(CMD_SETTRIG, 0, 0, &code, sizeof(code));
	}
}*/

/*
void Usb2020::setCalOutput(unsigned int index) const
{
	if (index <= 10)
	{
		unsigned char idx = index;
		sendCmd(CMD_CALCFG, 0, 0, &idx, 1, 1000);
	}
	else
		throw UlException(ERR_BAD_ARG);
}*/

} /* namespace ul */
