## MCC Universal Library for Linux (uldaq)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/16116/badge.svg)](https://scan.coverity.com/projects/uldaq)

**Info:** Contains a library to access and control supported Measurement Computing [DAQ devices](https://www.mccdaq.com/PDFs/Manuals/Linux-hw.pdf) over the Linux and macOS platforms. The UL for Linux binary name is libuldaq.

**Author:** Measurement Computing

## About
The **uldaq** package contains programming libraries and components for developing applications using C/C++ on Linux and macOS Operating Systems. An API (Application Programming Interface) for interacting with the library in Python is available as an additional installation. This package was created and is supported by MCC. 

### Prerequisites:
---------------
Building the **uldaq** package requires C/C++ compilers, make tool, and the development package for libusb. The following describes how these prerequisites can be installed on different Linux distributions and macOS.
  
  - Debian-based Linux distributions such as Ubuntu, Raspberry Pi OS
  
  ```
     $ sudo apt-get install gcc g++ make
     $ sudo apt-get install libusb-1.0-0-dev
  ```
  - Arch-based Linux distributions such as Manjaro, Antergos
  
  ```
     $ sudo pacman -S gcc make
     $ sudo pacman -S libusb
  ```
  - Red Hat-based Linux distributions such as Fedora, CentOS
  
  ```
     $ sudo yum install gcc gcc-c++ make
     $ sudo yum install libusbx-devel
  ``` 
  - OpenSUSE 
  
  ```
     $ sudo zypper install gcc gcc-c++ make
     $ sudo zypper install libusb-devel
  ```
  - macOS (Version 10.11 or later recommended)
  
  ```
     $ xcode-select --install
     $ ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
     $ brew install libusb
  ```

### Build Instructions
---------------------

1. Download the latest version of **uldaq**:

```
  Linux
     $ wget -N https://github.com/mccdaq/uldaq/releases/download/v1.2.1/libuldaq-1.2.1.tar.bz2

  macOS
     $ curl -L -O https://github.com/mccdaq/uldaq/releases/download/v1.2.1/libuldaq-1.2.1.tar.bz2
``` 
2. Extract the tar file:
 
```
  $ tar -xvjf libuldaq-1.2.1.tar.bz2
```
  
3. Run the following commands to build and install the library:

```
  $ cd libuldaq-1.2.1
  $ ./configure && make
  $ sudo make install
```

**Note:** To install the Python interface, follow the above [build instructions](#build-instructions) then go to https://pypi.org/project/uldaq/ for further installation.
  
### Examples
The C examples are located in the examples folder. Run the following commands to execute the analog input example: 

```
  $ cd examples
  $ ./AIn
```
Refer to the **uldaq** [PyPI](https://pypi.org/project/uldaq/) page for instructions on installing Python examples.

### Usage
The following is a basic C example of using the Universal Library for Linux to perform analog input. Further examples are available in the Examples folder.
``` C
#include <stdio.h>
#include "uldaq.h"

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64

int main(void)
{
	unsigned int numDevs = MAX_DEV_COUNT;
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceHandle handle = 0;

	int chan = 0;
	double data = 0;
	UlError err = ERR_NO_ERROR;

	// Get descriptors for all of the available DAQ devices
	ulGetDaqDeviceInventory(ANY_IFC, devDescriptors, &numDevs);
	
	// verify at least one DAQ device is detected
	if (numDevs)
	{
		// get a handle to the DAQ device associated with the first descriptor
		handle = ulCreateDaqDevice(devDescriptors[0]);

		// check if the DAQ device handle is valid
		if (handle)
		{
			// establish a connection to the DAQ device
			err = ulConnectDaqDevice(handle);

			// read data for the first 4 analog input channels
			for (chan = 0; chan <= 3; chan++)
			{
				err = ulAIn(handle, chan, AI_SINGLE_ENDED, BIP5VOLTS, AIN_FF_DEFAULT, &data);

				printf("Channel(%d) Data: %10.6f\n", chan, data);
			}

			ulDisconnectDaqDevice(handle);
			ulReleaseDaqDevice(handle);
		}
	}

	return 0;
}
```

### Uninstall Instructions
Run the following command to uninstall the library

``` sh
  $ sudo make uninstall
``` 

### Support/Feedback
The **uldaq** package is supported by MCC. For support for uldaq, contact technical support through [support page](https://www.mccdaq.com/support/support_form.aspx). Please include detailed steps on how to reproduce the problem in your request.

### Documentation
Online help for the Universal Library for Linux is available for [C/C++](https://www.mccdaq.com/PDFs/Manuals/UL-Linux/c/index.html) and [Python](https://www.mccdaq.com/PDFs/Manuals/UL-Linux/python/index.html).

If Doxygen is installed and you wish to build the API documentation on your system, run the following commands:

```
  $ cd doc
  $ doxygen Doxyfile
```

### Changelog
The changelog for this project can be found [here](https://github.com/mccdaq/uldaq/blob/master/CHANGELOG.md)

