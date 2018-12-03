/*
 * AoUsb3100.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef HID_AO_AOUSB3100_H_
#define HID_AO_AOUSB3100_H_

#include "AoHidBase.h"

namespace ul
{

class UL_LOCAL AoUsb3100: public AoHidBase
{
public:
	AoUsb3100(const HidDaqDevice& daqDevice);
	virtual ~AoUsb3100();

	virtual void initialize();

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);
	virtual void aOutArray(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[]);

	void configChanRange(int channel, Range range);

	virtual void setCfg_SyncMode(AOutSyncMode mode);
	virtual AOutSyncMode getCfg_SyncMode() const;

protected:
	void writeData(int channel, Range range, AOutFlag flags, double dataValue, unsigned char updateMode);
	int mapRangeCode(Range range) const;
	virtual int getCalCoefIndex(int channel, Range range) const;

private:
	int numChans() const;
	bool hasCurrentOutput() const;
	void loadDacCoefficients();

	AOutSyncMode readSyncMode() const;
	void writeSyncMode(AOutSyncMode mode) const;

private:
	enum { CMD_AOUT = 0x14, CMD_AOUTSYNC = 0x15, CMD_AOUTCONFIG = 0x1C, CMD_SETSYNC = 0x43, CMD_STATUS = 0x44};
	enum { UPDATE_IMMEDIATE = 0, UPDATE_ON_SYNC = 1 };

	Range mChanCurrentRange[16];
	AOutSyncMode mSyncMode;
};

} /* namespace ul */

#endif /* HID_AO_AOUSB3100_H_ */
