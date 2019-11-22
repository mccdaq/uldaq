/*
 * Usb1608hs.cpp
 *
 *      Author: Measurement Computing Corporation
 */

#include "Usb1608hs.h"
#include "./ai/AiUsb1608hs.h"
#include "./ao/AoUsb1608hs.h"
#include "./dio/DioUsb1608hs.h"
#include "./ctr/CtrUsb1208hs.h"

namespace ul
{
Usb1608hs::Usb1608hs(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	setCmdValue(CMD_STATUS_KEY, 0x40);
	setCmdValue(CMD_FLASH_LED_KEY, 0x41);
	setCmdValue(CMD_RESET_KEY, 0x42);

	mDaqDeviceInfo.setClockFreq(40000000);

	setAiDevice(new AiUsb1608hs(*this));
	setDioDevice(new DioUsb1608hs(*this));
	setCtrDevice(new CtrUsb1208hs(*this, 1));

	if(getDeviceType() == DaqDeviceId::USB_1608HS_2AO)
		setAoDevice(new AoUsb1608hs(*this, 2));

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

	addMemRegion(MR_CAL, 0x00, 314, MA_READ);
	addMemRegion(MR_USER, 0x140, 128, MA_READ | MA_WRITE);
}

Usb1608hs::~Usb1608hs()
{

}

unsigned char Usb1608hs::getTrigModeCode(TriggerType type) const
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
		case TRIG_ABOVE:
			code = LEVEL | HIGH << 1;
			break;
		case TRIG_LOW:
		case TRIG_BELOW:
			code = LEVEL | LOW << 1;
			break;
		default:
			throw UlException(ERR_BAD_TRIG_TYPE);
	}

	return code;
}

void Usb1608hs::setupTrigger(FunctionType functionType, ScanOption options) const
{
	if ((options & SO_EXTTRIGGER) || (options & SO_RETRIGGER))
	{
		TriggerConfig trigCfg = getTriggerConfig(functionType);

#pragma pack(1)
		struct
		{
			unsigned short value;
			unsigned char code;
		}cfg;
#pragma pack()

		unsigned short value = 0;

		if(trigCfg.type & (TRIG_POS_EDGE | TRIG_HIGH))
			value = TTLHIGH;
		else if(trigCfg.type & (TRIG_NEG_EDGE | TRIG_LOW))
			value = TTLLOW;
		else
			value =(trigCfg.level * (1 << 11) / 10) + 0x800;

		cfg.value = value;
		cfg.code = getTrigModeCode(trigCfg.type);

		sendCmd(CMD_SETTRIG, 0, 0, (unsigned char*)&cfg, sizeof(cfg));
	}
}

} /* namespace ul */
