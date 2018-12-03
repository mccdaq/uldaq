/*
 * Usb1608g.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "Usb1608g.h"
#include "./ai/AiUsb1608g.h"
#include "./ao/AoUsb1608g.h"
#include "./dio/DioUsb1608g.h"
#include "./ctr/CtrUsb1208hs.h"
#include "./tmr/TmrUsb1208hs.h"

namespace ul
{

Usb1608g::Usb1608g(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName) : UsbFpgaDevice(daqDeviceDescriptor, fpgaFileName)
{
	setCmdValue(CMD_STATUS_KEY, 0x40);
	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(64000000);

	setAiDevice(new AiUsb1608g(*this));
	setDioDevice(new DioUsb1608g(*this));
	setCtrDevice(new CtrUsb1208hs(*this, 2));
	setTmrDevice(new TmrUsb1208hs(*this, 1));

	if(getDeviceType() == DaqDeviceId::USB_1608GX_2AO || getDeviceType() == DaqDeviceId::USB_1608GX_2AO_2)
		setAoDevice(new AoUsb1608g(*this, 2));

	setOverrunBitMask(0x0004);
	setUnderrunBitMask(0x0010);
	setScanRunningBitMask(SD_INPUT, 0x0002);
	setScanRunningBitMask(SD_OUTPUT, 0x0008);
	setScanDoneBitMask(0x40);

	if(mDaqDeviceInfo.hasAoDevice())
		mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR | DE_ON_END_OF_OUTPUT_SCAN | DE_ON_OUTPUT_SCAN_ERROR);
	else
		mDaqDeviceInfo.setEventTypes(DE_ON_DATA_AVAILABLE | DE_ON_END_OF_INPUT_SCAN | DE_ON_INPUT_SCAN_ERROR);

	setMultiCmdMem(false);
	setMemUnlockAddr(0x8000);
	setMemUnlockCode(0xAA55);

	addMemRegion(MR_CAL, 0x7000, 256, MA_READ);
	addMemRegion(MR_USER, 0x7410, 3048, MA_READ | MA_WRITE);
}

Usb1608g::~Usb1608g()
{

}

unsigned char Usb1608g::getTrigModeCode(TriggerType type) const
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

void Usb1608g::setupTrigger(FunctionType functionType, ScanOption options) const
{
	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		TriggerConfig trigCfg = getTriggerConfig(functionType);

		unsigned char code = getTrigModeCode(trigCfg.type);

		sendCmd(CMD_SETTRIG, 0, 0, &code, sizeof(code));
	}
}

void Usb1608g::setCalOutput(unsigned int index) const
{
	if (index <= 10)
	{
		unsigned char idx = index;
		sendCmd(CMD_CALCFG, 0, 0, &idx, 1, 1000);
	}
	else
		throw UlException(ERR_BAD_ARG);
}

} /* namespace ul */
