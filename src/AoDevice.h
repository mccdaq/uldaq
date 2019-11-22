/*
 * AoDevice.h
 *
 *     Author: Measurement Computing Corporation
 */

#ifndef AODEVICE_H_
#define AODEVICE_H_

#include "ul_internal.h"
#include "IoDevice.h"
#include "AoInfo.h"
#include "AoConfig.h"
#include <vector>
#include "interfaces/UlAoDevice.h"

namespace ul
{

class UL_LOCAL AoDevice: public IoDevice, public UlAoDevice
{
public:
	AoDevice(const DaqDevice& daqDevice);
	virtual ~AoDevice();

	virtual const UlAoInfo& getAoInfo() { return mAoInfo;}
	virtual UlAoConfig& getAoConfig() { return *mAoConfig;}

	virtual void aOut(int channel, Range range, AOutFlag flags, double dataValue);
	virtual void aOutArray(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[]);
	virtual double aOutScan(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]);
	virtual void setTrigger(TriggerType type, int trigChan, double level, double variance, unsigned int retriggerCount);

	virtual UlError getStatus(ScanStatus* status, TransferStatus* xferStatus);
	virtual void stopBackground();

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual void setCfg_SyncMode(AOutSyncMode mode);
	virtual AOutSyncMode getCfg_SyncMode() const;

	virtual void setCfg_SenseMode(int channel, AOutSenseMode mode);
	virtual AOutSenseMode getCfg_SenseMode(int channel) const;

protected:
	virtual void loadDacCoefficients() = 0;
	virtual int getCalCoefIndex(int channel, Range range) const = 0;

	virtual CalCoef getCalCoef(int channel, Range range, long long flags) const;
	virtual CalCoef getDefaultCalCoef(int channel, Range range, long long flags) const;
	std::vector<CalCoef> getScanCalCoefs(int lowChan, int highChan, Range range, long long flags) const;

	virtual unsigned int calibrateData(int channel, Range range, AOutFlag flags, double data) const;

	double getMaxOutputValue(Range range, bool scaled) const;
	unsigned int fromEngUnits(double engUnits, Range range) const;
	double toEngUnits(unsigned int counts, Range range) const;

	void check_AOut_Args(int channel, Range range, AOutFlag flags, double dataValue) const;
	void check_AOutArray_Args(int lowChan, int highChan, Range range[], AOutArrayFlag flags, double data[]) const;
	void check_AOutScan_Args(int lowChan, int highChan, Range range, int samplesPerChan, double rate, ScanOption options, AOutScanFlag flags, double data[]) const;
	void check_AOutSetTrigger_Args(TriggerType trigType, int trigChan,  double level, double variance, unsigned int retriggerCount) const;

protected:
	AoInfo mAoInfo;
	AoConfig* mAoConfig;
	std::vector<CalCoef> mCalCoefs;

	unsigned long long mCalDate; // cal date in sec
};

} /* namespace ul */

#endif /* AODEVICE_H_ */
