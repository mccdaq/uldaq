/*
 * UsbIotech.cpp
 *
 * Author: Measurement Computing Corporation
 */

#include "UsbIotech.h"
#include "../utility/Endian.h"

namespace ul
{
UsbIotech::UsbIotech(const DaqDeviceDescriptor& daqDeviceDescriptor) : UsbDaqDevice(daqDeviceDescriptor)
{
	FnLog log("UsbIotech::UsbIotech");

	memset(&mDacCfg, 0, sizeof(mDacCfg));
}

UsbIotech::~UsbIotech()
{
	FnLog log("UsbIotech::~UsbIotech");

}

void UsbIotech::initilizeHardware() const
{
	// read fw version

	bool firstTime = true;

reinit:   // first time mimics when daqx library loads, the second time mimics when the daqx.open() function is invoked

	unsigned short version = 0;
	queryCmd(VR_GETFWVERSION, 0, 0, (unsigned char*) &version, sizeof(version), 2000);

	version = Endian::le_ui16_to_cpu(version);

	if(version >= 0x0100)
	{
		const_cast<UsbIotech*>(this)->mRawFwVersion = version;

		// from kernel code entryInitDevice()

		// test the FPGA to see if it is already in sync
		if(testMarkRegComm())
		{
			const_cast<UsbIotech*>(this)->initializeDac();

			const_cast<UsbIotech*>(this)->initializeAdc();

			if(firstTime)
			{
				const_cast<UsbIotech*>(this)->uninitializeDac();
				const_cast<UsbIotech*>(this)->uninitializeAdc();

				firstTime = false;
				goto reinit;
			}
		}

	}
	else
	{
		std::cout << "invalid fw version" << std::endl;
	}

}

bool UsbIotech::testMarkRegComm() const
{
	bool passed = true;
	int	i;

	// write a walking and verify it can be read
	for( i = 0; i < 8; i++ )
	{
		unsigned short byteWritten, byteRead;

		// create the test pattern
		byteWritten = ((i & 1) ? (1 << i) : ~(1 << i)) & 0x00FF;

		// write the test byte to what should be the low byte
		// and a zero to what should be the high byte
		sendCmd(VR_FPGA_REGIO, byteWritten, HWRegDigitalMark, NULL, 0);

		// read value back and verify it's equal to the test value
		// set the return value to false if not equal
		queryCmd(VR_FPGA_REGIO, 0, HWRegDigitalMark, (unsigned char*) &byteRead, sizeof(byteRead));

		byteRead = Endian::le_ui16_to_cpu(byteRead);

		// verify that the value written was read properly
		if( byteWritten != (byteRead & 0x00FF) )
		{
			printf("entryTestBaseAddressValid: ERROR...test loop, ByteWritten=0x%02x, ByteRead=0x%02x\n", byteWritten, byteRead);
			passed = false;

			// if it failed, no sense in hanging around in the loop anymore
			break;
		}

	}

	//DbgPrintf("entryTestBaseAddressValid: Info...%u FPGA 16-bit register tests completed\n", i);

	return passed;
}


void UsbIotech::initializeDac()
{
	const_cast<UsbIotech*>(this)->mDacCfg.dacPacerClockSelection = 0;
	memset((void*)(mDacCfg.enableBits), 0, sizeof(mDacCfg.enableBits));

	// disarm the hardware
	dacDisarm();

	// Set the dac enable bits, default to unsigned data
	mDacCfg.enableBits[0] = Dac0Enable;
	mDacCfg.enableBits[1] = Dac1Enable;
	mDacCfg.enableBits[2] = Dac2Enable;
	mDacCfg.enableBits[3] = Dac3Enable;
	mDacCfg.enableBits[4] = DacPatternEnable;

	// Reset the DAC FIFO
	sendCmd(VR_FPGA_REGIO, DacResetFifo, HWRegDacControl, NULL, 0);
}

void UsbIotech::uninitializeDac()
{
	// disarm the hardware
	dacDisarm();
}

void UsbIotech::dacDisarm()
{
	int i;

	// Clear "enable dac pacer at same time as adc pacer" flag
	const_cast<UsbIotech*>(this)->mDacCfg.dacTriggerIsAdc = false;

	// Stop the pacer clock
	sendCmd(VR_FPGA_REGIO, (mDacCfg.dacPacerClockSelection | DacPacerDisable), HWRegDacControl, NULL, 0);

	// Disable DMA Channel 0 transfers (via fpga)
	sendCmd(VR_FPGA_REGIO, DmaCh0Disable, HWRegDmaControl, NULL, 0);

	// Disable the dacs
	for(i = 0; i < 5; i++)
	{
		// Disable next dac
		sendCmd(VR_FPGA_REGIO, (mDacCfg.enableBits[i] & ~DacEnableBit), HWRegDacControl, NULL, 0);
	}
}

void UsbIotech::initializeAdc()
{
	// disarm the hardware
	adcDisarm();
}

void UsbIotech::uninitializeAdc()
{
	// disarm the hardware
	adcDisarm();
}

void UsbIotech::adcDisarm()
{
	// Disable hardware triggers
	sendCmd(VR_FPGA_REGIO, TrigAnalog | TrigDisable, HWRegTrigControl, NULL, 0);
	sendCmd(VR_FPGA_REGIO, TrigTTL | TrigDisable, HWRegTrigControl, NULL, 0);

	// Stop the pacer clock
	sendCmd(VR_FPGA_REGIO, AdcPacerDisable, HWRegAcqControl, NULL, 0);

	// Stop the scan list FIFO from loading the configuration pipe
	sendCmd(VR_FPGA_REGIO, SeqStopScanList, HWRegAcqControl, NULL, 0);

	sendCmd(VR_FPGA_REGIO, AcqResetResultsFifo|AcqResetConfigPipe, HWRegAcqControl, NULL, 0);

	// Disable DMA Channel 1 transfers (via fpga)
	sendCmd(VR_FPGA_REGIO, DmaCh1Disable, HWRegDmaControl, NULL, 0);
}

void UsbIotech::flashLed(int flashCount) const
{
	throw UlException(ERR_BAD_DEV_TYPE);
}



} /* namespace ul */
