/*
 * Usb1208hs.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "Usb1208hs.h"
#include "./ai/AiUsb1208hs.h"
#include "./ao/AoUsb1208hs.h"
#include "./dio/DioUsb1208hs.h"
#include "./ctr/CtrUsb1208hs.h"
#include "./tmr/TmrUsb1208hs.h"

namespace ul
{

Usb1208hs::Usb1208hs(const DaqDeviceDescriptor& daqDeviceDescriptor, std::string fpgaFileName) : UsbFpgaDevice(daqDeviceDescriptor, fpgaFileName)
{
	setCmdValue(CMD_STATUS_KEY, 0x40);
	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(40000000);

	setAiDevice(new AiUsb1208hs(*this));
	setDioDevice(new DioUsb1208hs(*this));
	setCtrDevice(new CtrUsb1208hs(*this, 2));
	setTmrDevice(new TmrUsb1208hs(*this, 1));

	if(getDeviceType() == DaqDeviceId::USB_1208HS_2AO)
		setAoDevice(new AoUsb1208hs(*this, 2));
	else if(getDeviceType() == DaqDeviceId::USB_1208HS_4AO)
		setAoDevice(new AoUsb1208hs(*this, 4));

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

	addMemRegion(MR_CAL, 0x4000, 0x1000, MA_READ);
	addMemRegion(MR_USER, 0x5000, 0x3000, MA_READ | MA_WRITE);

	setMinRawFwVersion(0x107); // minimum supported firmware version is 1.07
}

Usb1208hs::~Usb1208hs()
{

}

unsigned char Usb1208hs::getTrigModeCode(TriggerType type) const
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

void Usb1208hs::setupTrigger(FunctionType functionType, ScanOption options) const
{
	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		TriggerConfig trigCfg = getTriggerConfig(functionType);

		unsigned char code = getTrigModeCode(trigCfg.type);

		sendCmd(CMD_SETTRIG, 0, 0, &code, sizeof(code));
	}
}

} /* namespace ul */
