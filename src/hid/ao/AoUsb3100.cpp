/*
 * AoUsb3100.cpp
 *
 *     Author: Measurement Computing Corporation
 */

#include "AoUsb3100.h"

namespace ul
{

AoUsb3100::AoUsb3100(const HidDaqDevice& daqDevice) : AoHidBase(daqDevice)
{
	mAoInfo.setAOutFlags(AOUT_FF_NOSCALEDATA | AOUT_FF_NOCALIBRATEDATA);
	mAoInfo.setAOutArrayFlags(AOUTARRAY_FF_NOSCALEDATA | AOUTARRAY_FF_NOCALIBRATEDATA | AOUTARRAY_FF_SIMULTANEOUS);

	mAoInfo.setNumChans(numChans());
	mAoInfo.setResolution(16);

	mAoInfo.setCalCoefsStartAddr(0x100);
	mAoInfo.setSampleSize(2);

	mAoInfo.addRange(BIP10VOLTS);
	mAoInfo.addRange(UNI10VOLTS);
	int rangeCount = 2;

	if(hasCurrentOutput())
	{
		mAoInfo.addRange(MA0TO20);
		rangeCount++;
	}

	mAoInfo.setCalCoefCount(rangeCount * mAoInfo.getNumChans());

	mSyncMode = AOSM_SLAVE;

	memset(mChanCurrentRange, 0, sizeof(mChanCurrentRange));
}

AoUsb3100::~AoUsb3100()
{

}

void AoUsb3100::initialize()
{
	memset(mChanCurrentRange, 0, sizeof(mChanCurrentRange));

	try
	{
		mSyncMode = readSyncMode();

		loadDacCoefficients();
	}
	catch(UlException& e)
	{
		UL_LOG("Ul exception occurred: " << e.what());
	}
}

void AoUsb3100::aOut(int channel, Range range, AOutFlag flags, double dataValue)
{
	check_AOut_Args(channel, range, flags, dataValue);

	writeData(channel, range, flags, dataValue, UPDATE_IMMEDIATE);
}

void AoUsb3100::aOutArray(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[])
{
	check_AOutArray_Args(lowChan, highChan, range, flags, data);

	unsigned char updateMode = (flags & AOUTARRAY_FF_SIMULTANEOUS) ? UPDATE_ON_SYNC : UPDATE_IMMEDIATE;

	int i = 0;
	for(int chan = lowChan; chan <= highChan; chan++)
	{
		writeData(chan, range[i], (AOutFlag) flags, data[i], updateMode);
		i++;
	}

	if(flags & AOUTARRAY_FF_SIMULTANEOUS)
	{
		if(mSyncMode == AOSM_MASTER)
			daqDev().sendCmd(CMD_AOUTSYNC);
	}
}

void AoUsb3100::writeData(int channel, Range range, AOutFlag flags, double dataValue, unsigned char updateMode)
{
	if(mChanCurrentRange[channel] != range)
		configChanRange(channel, range);

	unsigned char chan = channel;
	unsigned short calData = calibrateData(channel, range, flags, dataValue);

	if(range == MA0TO20 && dataValue == 0.0) // don't apply cal factors if output value is zero, from the windows code
		calData = 0;

	calData = Endian::cpu_to_le_ui16(calData);

	daqDev().sendCmd(CMD_AOUT, chan, calData, updateMode);
}


void AoUsb3100::configChanRange(int channel, Range range)
{
	unsigned char rangeCode = mapRangeCode(range);

	daqDev().sendCmd(CMD_AOUTCONFIG, (unsigned char) channel , rangeCode);

	mChanCurrentRange[channel] = range;
}

AOutSyncMode AoUsb3100::readSyncMode() const
{
	unsigned char cmd = CMD_STATUS;
	unsigned char status = 0;

	daqDev().queryCmd(cmd, &status);

	AOutSyncMode mode = (AOutSyncMode)(status & 0x01);

	return mode;
}

void AoUsb3100::writeSyncMode(AOutSyncMode mode) const
{
	unsigned char cmd = CMD_SETSYNC;
	unsigned char type = (mode == 0) ? 1 : 0;

	daqDev().sendCmd(cmd, type);
}

int AoUsb3100::mapRangeCode(Range range) const
{
	int rangeCode;

	switch(range)
	{
	case UNI10VOLTS:
		rangeCode = 0;
		break;
	case BIP10VOLTS:
		rangeCode = 1;
		break;
	case MA0TO20:
		rangeCode = 0;
		break;
	default:
		throw UlException(ERR_BAD_RANGE);
	}

	return rangeCode;
}

int AoUsb3100::getCalCoefIndex(int channel, Range range) const
{
	int index = 0;

	if(range == UNI10VOLTS)
		index = channel * 2;
	else if(range == BIP10VOLTS)
		index = channel * 2 + 1;
	else if(range == MA0TO20)
		index =  mAoInfo.getNumChans() * 2 + channel;

	return index;
}

void AoUsb3100::loadDacCoefficients()
{
#pragma pack(1)
	typedef struct
	{
		unsigned char slope[4];
		unsigned char offset[4];
	}  coef;
#pragma pack()

	CalCoef calCoef;

	mCalCoefs.clear();

	bool readCurOutCoefs = false;
	int calCoefCount = mAoInfo.getNumChans() * 2;
	int calBlockSize = calCoefCount * sizeof(coef);
	int address = mAoInfo.getCalCoefsStartAddr();

	coef* buffer = new coef[calCoefCount];

start:

	int bytesReceived = daqDev().memRead(MT_EEPROM, MR_CAL, address, (unsigned char*)buffer, calBlockSize);

	if(bytesReceived == calBlockSize)
	{
		for(int i = 0; i < calCoefCount; i++)
		{
			calCoef.slope = mEndian.le_ptr_to_cpu_f32(buffer[i].slope);
			calCoef.offset = mEndian.le_ptr_to_cpu_f32(buffer[i].offset);

			mCalCoefs.push_back(calCoef);
		}
	}

	if(hasCurrentOutput() && !readCurOutCoefs)
	{
		calCoefCount = mAoInfo.getNumChans();
		calBlockSize = calCoefCount * sizeof(coef);
		address = 0x200;
		readCurOutCoefs = true;
		goto start;
	}

	delete [] buffer;
}


int AoUsb3100::numChans() const
{
	int numChans;
	switch(daqDev().getDeviceType())
	{
	case DaqDeviceId::USB_3103:
	case DaqDeviceId::USB_3104:
	case DaqDeviceId::USB_3112:
		numChans = 8;
		break;
	case DaqDeviceId::USB_3105:
	case DaqDeviceId::USB_3106:
	case DaqDeviceId::USB_3114:
			numChans = 16;
		break;
	default:
		numChans = 4;
		break;
	}

	return numChans;
}

bool AoUsb3100::hasCurrentOutput() const
{
	bool hasCurOut = false;

	if(daqDev().getDeviceType() == DaqDeviceId::USB_3102 ||
		daqDev().getDeviceType() == DaqDeviceId::USB_3104 ||
		daqDev().getDeviceType() == DaqDeviceId::USB_3106)
	{
		hasCurOut = true;
	}

	return hasCurOut;
}

void AoUsb3100::setCfg_SyncMode(AOutSyncMode mode)
{
	writeSyncMode(mode);

	mSyncMode = readSyncMode();
}
AOutSyncMode AoUsb3100::getCfg_SyncMode() const
{
	return readSyncMode();
}


} /* namespace ul */
