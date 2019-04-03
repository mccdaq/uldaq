/*
 * UsbIotech.h
 *
 *  Author: Measurement Computing Corporation
 */

#ifndef USB_USBIOTECH_H_
#define USB_USBIOTECH_H_

#include "UsbDaqDevice.h"

namespace ul
{

// Prepare for acquisition
#define AcqResetScanListFifo        0x0004
#define AcqResetResultsFifo         0x0002
#define AcqResetConfigPipe          0x0001
#define AcqFifoFill                 0x0002

// Acqusition status bits
#define AcqResultsFIFOMore1Sample   0x0001
#define AcqResultsFIFOHasValidData  0x0002
#define AcqResultsFIFOOverrun       0x0004
#define AcqLogicScanning            0x0008
#define AcqConfigPipeFull           0x0010
#define AcqScanListFIFOEmpty        0x0020
#define AcqAdcNotReady              0x0040
#define ArbitrationFailure          0x0080
#define AcqPacerOverrun             0x0100
#define DacPacerOverrun             0x0200
#define AcqHardwareError            0x01c0

// Pacer Clock Control
#define AdcPacerInternal            0x0030
#define AdcPacerExternal            0x0032
#define AdcPacerEnable              0x0031
#define AdcPacerEnableDacPacer      0x0034
#define AdcPacerDisable             0x0030
#define AdcPacerNormalMode          0x0060
#define AdcPacerCompatibilityMode   0x0061
#define AdcPacerInternalOutEnable   0x0008
#define AdcPacerExternalRising      0x0100
#define AdcPacer1604                0x0200

//Setpoint Control
#define SetpointFIFOReset			0x0054
#define SetpointFIFOStart			0x0071

//Digital Enhancement bits 15:8 are XOR amount 2 to 256
#define DigEnhDisable				0x0040
#define DigEnhStartOfSample			0x0041
#define DigEnhOverSample			0x0042

// Scan Sequencer programming
#define SeqStartScanList            0x0011
#define SeqStopScanList             0x0010

// DAC control
#define Dac0Enable                  0x0021
#define Dac1Enable                  0x0031
#define Dac2Enable                  0x0041
#define Dac3Enable                  0x0051
#define DacEnableBit                0x0001
#define Dac0Disable                 0x0020
#define Dac1Disable                 0x0030
#define Dac2Disable                 0x0040
#define Dac3Disable                 0x0050
#define DacResetFifo                0x0004
#define DacPatternDisable           0x0060
#define DacPatternEnable            0x0061
#define DacSelectSignedData         0x0002
#define DacSelectUnsignedData       0x0000

// Dac Pacer Clock Control
#define DacPacerInternal            0x0010
#define DacPacerExternal            0x0012
#define DacPacerEnable              0x0011
#define DacPacerDisable             0x0010
#define DacPacerUseAdc              0x0014
#define DacPacerInternalOutEnable   0x0008
#define DacPacerExternalRising      0x0100

// Trigger Control
#define TrigAnalog                  0x0000
#define TrigTTL                     0x0010
#define TrigTransHiLo               0x0004
#define TrigTransLoHi               0x0000
#define TrigAbove                   0x0000
#define TrigBelow                   0x0004
#define TrigLevelSense              0x0002
#define TrigEdgeSense               0x0000
#define TrigEnable                  0x0001
#define TrigDisable                 0x0000

// Dma Control
#define DmaCh0Enable                0x0001
#define DmaCh0Disable               0x0000
#define DmaCh1Enable                0x0011
#define DmaCh1Disable               0x0010

// P3 HSIO Control
#define p3HSIOIsInput               0x0050
#define p3HSIOIsOutput              0x0052

// P2 Control
#define p2Is82c55                   0x0030
#define p2IsExpansionPort           0x0032

// endpoint control definitions
#define EPC_RESET       			0x0001

class UL_LOCAL UsbIotech: public UsbDaqDevice
{
public:
	UsbIotech(const DaqDeviceDescriptor& daqDeviceDescriptor);
	virtual ~UsbIotech();

	virtual void flashLed(int flashCount) const;

private:
	virtual void initilizeHardware() const;

	bool testMarkRegComm() const;

	void initializeDac();
	void uninitializeDac();
	void dacDisarm();

	void initializeAdc();
	void uninitializeAdc();
	void adcDisarm();


private:

	struct
	{
		unsigned short dacPacerClockSelection;
		bool dacTriggerIsAdc;
		unsigned short  enableBits[5]; // Reset and enable bits for this dac
	} mDacCfg;



public:
	enum { VR_GETFWVERSION = 0xb0, /* read the firmware version */
		   VR_FPGA_REGIO = 0xb4, /* read/write fpga register*/
		   VR_EP_CONTROL = 0xbb};

	enum {
			HWRegAcqControl               = 0x00,
			HWRegAcqStatus                = 0x00,
			HWRegAcqScanListFIFO          = 0x01,
			HWRegAcqPacerClockDivLow      = 0x02,
			HWRegAcqPacerClockDivMed      = 0x03,
			HWRegAcqScanCounter           = 0x04, // DaqBoard/2000 series only
			HWRegAcqPacerClockDivHigh     = 0x05,
			HWRegPDQUtility               = 0x07,
			HWRegAcqResultsFIFOLow        = 0x08, // DaqBoard/2000 series only
			HWRegSetpointFIFO			  = 0x09,
			HWRegAcqResultsShadow         = 0x0a, // DaqBoard/2000 series only
			HWRegSetpointResult			  = 0x0b,
			HWRegAcqAdcResult             = 0x0c,
			HWRegDacScanCounter           = 0x0e, // DaqBoard/2000 series only
			HWRegVariableConvRate		  = 0x0f,
			HWRegDacControl               = 0x10,
			HWRegDacStatus                = 0x10,
			HWRegDacPacerClockDivHigh     = 0x11,
			HWRegDacFIFOLow               = 0x12, // DaqBoard/2000 series only
			HWRegDacPacerClockDivLow      = 0x15,
			HWRegRefDacs                  = 0x16,
			HWRegDacPacerClockDivMed      = 0x17,
			HWRegDioControl               = 0x18,
			HWRegDioP3hsioData            = 0x19,
			HWRegCalEepromControl         = 0x1b,
			HWRegDacSetting               = 0x1c, // 4 dacs (0x1c-0x1f)
			HWRegDioP2IO8Bit              = 0x20, // 1 8255 (0x20-0x23) or 32 expansion (0x20-0x3f)
			HWRegCtrEnhSetup			  = 0x28, // 4 enhanced wbk17-style counters (0x28-0x2B)
			HWRegCtrEnhDebounce			  = 0x2C, // 4 single byte setup values for enhanced wbk17-style counters(0x2C-0x2D)
			HWRegCtrEnhBankSelect		  = 0x2E, // bank select for multi-wbk17 style counter implementations (USB160x)
			HWRegCtrEnhRead				  = 0x30, // 4 wbk17-style ctrs (alternating low/high word) (0x30-0x37)
			HWRegCtrTmrControl            = 0x40,
			HWRegCtrInput                 = 0x44, // 4 counters (0x44-0x47)
			HWRegTimerDivisor             = 0x50, // 2 timer divisors (0x50-0x51)
			HWRegDmaControl               = 0x58, // DaqBoard/2000 series only
			HWRegTrigControl              = 0x59,
			HWRegTrigStatus               = 0x59,
			HWRegCalDac                   = 0x5a,
			HWRegCalEeprom                = 0x5c,
			HWRegDigitalMark              = 0x5d, // DaqBook/2000 series only???
			HWRegTriggerDacs              = 0x5e,
			HWRegTempSensor               = 0x5f,
			HWRegExtClockDiv              = 0x74, // DaqBook/2000 series only
			HWRegMasterClock              = 0x76, // DaqBook/2000 series only
			HWRegFIFOWrite                = 0x77, // DaqBook/2000 series only
			HWRegCommand                  = 0x78, // DaqBook/2000 series only
			HWRegCommandData              = 0x79, // DaqBook/2000 series only
			HWRegFIFOStatus               = 0x7a, // DaqBook/2000 series only
			HWRegPerfTest                 = 0x7b, // DaqBook/2000 series only
			HWRegFIFOLow                  = 0x7c, // DaqBook/2000 series only
			HWRegFIFOHigh                 = 0x7d, // DaqBook/2000 series only
			HWRegFIFOAlt                  = 0x7e, // DaqBook/2000 series only, DaqBoard3K uses this for a DMA DWORD counter
			HWRegInterruptControl         = 0x7f, // DaqBook/2000 series only
			HWReg16BitRegCuttoff          = 0x80, // No 8-bit registers
	};

};

} /* namespace ul */

#endif /* USB_USBIOTECH_H_ */
