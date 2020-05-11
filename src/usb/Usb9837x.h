/*
 * Usb9837x.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef USB_USB9837X_H_
#define USB_USB9837X_H_

#include "UsbDtDevice.h"
#include "./dt/Usb9837xDefs.h"

// TI ADS1271 hardware
#define ADS1271_LOW_POWER_UPPER_FREQ	52734.0
#define ADS1271_HIGH_SPEED_UPPER_FREQ	105469.0

namespace ul
{

class UL_LOCAL Usb9837x: public UsbDtDevice
{
public:
	Usb9837x(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~Usb9837x();

	virtual void disconnect();

	static void readIdentifier(libusb_device* dev, libusb_device_descriptor descriptor, unsigned int* identifier);
	static void readSerialNumber(libusb_device* dev, libusb_device_descriptor descriptor, char* serialNum);
	static unsigned int getVirtualProductId(libusb_device* dev, libusb_device_descriptor descriptor);

	bool isPowerAlwaysOn() const;
	void writePowerAlwaysOnToEeprom() const;
	void cmdPowerDevice( bool bPowerOn ) const;

	virtual void messageHandler(const unsigned char* messageBuffer);


private:
	virtual void initilizeHardware() const;


public:
	/*void Cmd_ReadUsbReg( unsigned char Address, unsigned char *DataVal ) const;
	void Cmd_WriteUsbReg( unsigned char Address, unsigned char DataVal ) const;
	void Cmd_ReadMultipleUsbReg( unsigned char NumReads, unsigned char *Addresses, unsigned char *pData ) const;
	void Cmd_WriteMultipleUsbReg( unsigned char NumWrites, WRITE_BYTE_INFO *pWrite  ) const;
	void Cmd_ModifyUsbReg( unsigned char Address, unsigned char AndMask, unsigned char OrVal ) const;
	void Cmd_ModifyMultipleUsbReg( unsigned char NumModifies, RMW_BYTE_INFO *ByteInfo ) const;*/

	void Cmd_ReadSingleWordFromLocalBus( unsigned short Address, unsigned short *pDataVal ) const;
	void Cmd_WriteSingleWordToLocalBus( unsigned short Address, unsigned short DataVal ) const;
	void Cmd_RMWSingleWordToLocalBus( unsigned short Address, unsigned short AndMask, unsigned short OrVal ) const;

	void Cmd_StartSubsystem (Usb9837xDefs::pSUBSYSTEM_INFO pSubsystemInfo) const;
	//void Cmd_SimStartSubsystem (pSUBSYSTEM_INFO pSubsystemInfo) const;
	void Cmd_StopSubsystem (Usb9837xDefs::pSUBSYSTEM_INFO pSubsystemInfo) const;

	void Cmd_ReadDevMultipleRegs( unsigned char DevAddress, unsigned char NumRegs, unsigned char *pRegisters, unsigned char *pData ) const;
	void Cmd_WriteDevMultipleRegs( unsigned char DevAddress, unsigned char NumRegs, unsigned char *pRegisters, unsigned char *pData ) const;
	void Cmd_WriteMultiplePLLReg(Usb9837xDefs::SUBSYSTEM_TYPE  SubsystemType, unsigned char DevAddress, unsigned char NumWrites, Usb9837xDefs::WRITE_BYTE_INFO* pWrite ) const;

	void Cmd_WriteSingleValue (Usb9837xDefs::pWRITE_SINGLE_VALUE_INFO pPutSingleValueInfo) const;
	void Cmd_ReadSingleValue (Usb9837xDefs::pREAD_SINGLE_VALUE_INFO pReadSingleValueInfo, unsigned int* pData) const;
	//void Cmd_ReadSingleValues (unsigned char NumChans, unsigned char gain, DWORD *pData) const;
	void Cmd_SetDaFifoSize(unsigned int dacFifoSize) const;

	void CmdSetAnalogTriggerThreshold( double volts) const;

	static void programClock(double FreqIn,double FreqRef ,double* actualFreq, Usb9837xDefs::CY22150REGISTERS* CY22150Registers, double* fOut, unsigned char* pDivider,bool isDac);

	static void  FindVCOfreq (double infreq, double* vcofreq, int* DivIn);
	static int FindQMax(double freqref);
	static void FindDivRatioIn (double freq, int* DivIn);
	static double PdivByQ(double vcofreq, double fref);
	static void optimumPQF(double desiredfreq, double Pdiv_Q, double fref, int qmax, int* Popt, int* Qopt, double* fout, int* errorout, int* count);
	static void CalculatePQValues(int PTotal, int QTotal, int* P, int* Q);
	static int FindChargePumpSetting(int PVal);
};

} /* namespace ul */

#endif /* USB_USB9837X_H_ */
