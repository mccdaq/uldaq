Name: uldaq
Version: 1.1.0
Release: 1%{?dist}
Summary: MCC Universal Library for Linux
URL: http://www.mccdaq.com
License: MIT

Source0: https://github.com/mccdaq/uldaq/archive/v1.1.0.tar.gz
#Patch0: add-libuldaq.pc.in.patch
#Patch1: fix-debug.patch
#Patch2: fix-debug2.patch

BuildRequires: libusb-devel m4 autoconf automake libtool
Requires: libusb

%description
The uldaq package contains programming libraries and components for developing applications using C/C++ on Linux and macOS Operating Systems.
An API (Application Programming Interface) for interacting with the library in Python is available as an additional installation.
This package was created and is supported by MCC.

%package devel
Summary: Headers and libraries for building apps that use uldaq
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
This package contains headers and libraries required to build applications that
use the uldaq acquisition devices.

%prep
%setup -q
%patch0 -p0
%patch1 -p1
%patch2 -p1
autoreconf -i

%build
CFLAGS="-fPIC"; export CFLAGS
LDFLAGS="-fPIC"; export LDFLAGS
%configure --prefix=/usr --enable-shared --enable-debug
make %{_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

%files
%defattr(-,root,root,-)
%doc README.md
%{_defaultdocdir}/libuldaq/README.md

%{_libdir}/libuldaq.so.1.1.0
%{_libdir}/libuldaq.so.1
%{_libdir}/libuldaq.so

/etc/uldaq/fpga/*
/lib/udev/rules.d/50-uldaq.rules

%files devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/libuldaq.pc
%{_includedir}/uldaq.h
%{_libdir}/libuldaq.a
%{_libdir}/libuldaq.la

%changelog
* Thu Jan 10 2019 Steffen Vogel <post@steffenvogel.de> 1.1.0-1
- Created Initial Spec File.
