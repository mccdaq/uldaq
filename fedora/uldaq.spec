Name:           uldaq
Version:        1.1.2
Release:        1%{?dist}
Summary:        Programming libraries for interfacing with a MCC usb DAQ

Group:          System Environment/Daemons
License:        MIT
URL:            http://https://github.com/mccdaq/uldaq
Source0:        https://github.com/mccdaq/%{name}/releases/download/v%{version}/lib%{name}-%{version}.tar.bz2
Patch0:        uldaq_ldconfig.patch
Vendor:         Measurement Computing Corporation

BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	libusb-devel
Requires:	libusb

%description
The uldaq package contains programming libraries and components for
developing applications using C/C++ on Linux and macOS Operating Systems.
An API (Application Programming Interface) for interacting with the library
in Python is available as an additional installation.

%prep
tar -xjvf  %{_sourcedir}/lib%{name}-%{version}.tar.bz2
cd lib%{name}-%{version} 
%patch0 -p1

%build
cd lib%{name}-%{version}
autoreconf -if
./configure --prefix="/usr" --libdir="/usr/lib64"
make

%install
cd lib%{name}-%{version}
make DESTDIR=%{buildroot} install 

%post
udevadm control --reload-rules
ldconfig

%preun

%postun

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
#%dir %attr(750, root, root) %{_sysconfdir}/%{name}
%{_includedir}/%{name}.h
%{_libdir}/libuldaq.*
%{_libdir}/pkgconfig/lib%{name}.pc
%{_docdir}/lib%{name}/README.md
/lib/udev/rules.d/50-%{name}.rules
%{_datadir}/%{name}/fpga

%changelog
* Fri Dec 6 2019 Joshua Clayton <joshua.clayton@3deolidar.com> - 1.1.2-1
- First release 
