/*
 * DioUsbTc32.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_DIO_DIOUSBTC32_H_
#define USB_DIO_DIOUSBTC32_H_

#include "DioUsbBase.h"

namespace ul
{

class UL_LOCAL DioUsbTc32: public DioUsbBase
{
public:
	DioUsbTc32(const UsbDaqDevice& daqDevice);
	virtual ~DioUsbTc32();

	virtual void initialize();

	virtual unsigned long long dIn(DigitalPortType portType);
	virtual void dOut(DigitalPortType portType, unsigned long long data);

	virtual bool dBitIn(DigitalPortType portType, int bitNum);
	virtual void dBitOut(DigitalPortType portType, int bitNum, bool bitValue);

	virtual void dClearAlarm(DigitalPortType portType, unsigned long long mask);

	//////////////////////          Configuration functions          /////////////////////////////////
	virtual unsigned long long getCfg_PortDirectionMask(unsigned int portNum) const;

protected:
	void readAlarmMask();

private:
	std::bitset<32> mAlarmMask[2];
	enum {CMD_DIN = 0x00, CMD_DOUT = 0x02, CMD_ALARM_CONFIG = 0x20, CMD_ALARM_STATUS = 0x22};
};

} /* namespace ul */

#endif /* USB_DIO_DIOUSBTC32_H_ */
