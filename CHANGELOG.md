# Changelog
All notable changes to the UL for Linux project will be documented in this file.

## [1.2.1] - March 21, 2022
### Added
- none

### Changed
- The FPGA image of the USB-2020 series upgraded to version 0.92. This update addresses the digital output update issue.
- Removed compile warnings.
- Updated the FPGA loader.

### Removed
- none

## [1.2.0] - May 11, 2020
### Added
* support for the following DAQ devices
  * E-1608
  * E-TC
  * E-DIO24
  * TC32

* RemoteNetDiscovery example
  
### Changed
- The FPGA folder and its content have been removed. The FPGA images have been embedded in the library file.
- To address the libusb fork operation issue which is explained [here](http://libusb.sourceforge.net/api-1.0/libusb_caveats.html), libusb will no longer be initialized when the uldaq library is loaded. Instead, libusb is initialized when the first uldaq function is invoked. This change allows users to initialize libusb in the child process by invoking the first uldaq function after a fork() call. 

### Removed
- none

## [1.1.2] - November 22, 2019
### Added
* support for the following DAQ devices
  * USB-2408
  * USB-2408-2AO
  * USB-2416
  * USB-2416-4AO
  * USB-1608HS
  * USB-1608HS-2AO
  * USB-2020
  * USB-2001-TC

### Changed
- The FPGA image of the USB-1208HS series upgraded to version 1.05. This update addresses the analog output channel rotation issue.

### Removed
- none

## [1.1.1] - April 3, 2019
### Added
* support for the following DAQ devices
  * USB-QUAD08
  * DT9837A
  * DT9837B
  * DT9837C

* AInScanIepe.c example
  
### Changed
- none

### Removed
- none

## [1.1.0] - Dec 3, 2018
### Added
* support for the HID DAQ devices
* support for the macOS platform

### Changed
- none

### Removed
- none

## [1.0.0] - May 15, 2018
Initial release
