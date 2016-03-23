Name: toybox
Version: 0.6.0
Release: 1%{?dist}
Summary: Single binary providing simplified versions of system commands
Group: Base/Utilities
License: BSD-2.0
URL: http://www.landley.net/toybox/
Source: %{name}-%{version}.tar.bz2
Source1: config
%if "%{?profile}"=="tv"
Source2: bin_tv.links
Source3: sbin_tv.links
%else
Source2: bin.links
Source3: sbin.links
%endif
Source4: usrbin.links
Source5: usrsbin.links
Source1001: toybox.manifest

BuildRequires : smack-devel
BuildRequires : libattr-devel

%description
Toybox is a single binary which includes versions of a large number
of system commands, including a shell.  This package can be very
useful for recovering from certain types of system failures,
particularly those involving broken shared libraries.

%package symlinks-dhcp
Group: Base/Utilities
Summary: ToyBox symlinks to provide 'dhcp'
Requires: %{name} = %{version}-%{release}

%description symlinks-dhcp
ToyBox symlinks for utilities corresponding to 'dhcp' package.

%package symlinks-dhcpd
Group: Base/Utilities
Summary: ToyBox symlinks to provide 'dhcpd'
Requires: %{name} = %{version}-%{release}

%description symlinks-dhcpd
ToyBox symlinks for utilities corresponding to 'dhcpd' package.

%prep
%setup -q

%build
cp %{SOURCE1001} .
# create dynamic toybox - the executable is toybox
cp %{SOURCE1} .config
make -j 4 CC="gcc $RPM_OPT_FLAGS" CFLAGS="$CFLAGS -fPIE" LDOPTIMIZE="-Wl,--gc-sections -pie"
cp toybox toybox-dynamic

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/sbin
install -m 755 toybox-dynamic $RPM_BUILD_ROOT/bin/toybox

# debian/toybox.links
pushd %{buildroot}
mkdir -p usr/bin usr/sbin sbin
cd bin
for f in `cat %SOURCE2` ; do ln -s toybox $f ; done
cd ../sbin
for f in `cat %SOURCE3` ; do ln -s ../bin/toybox $f ; done
cd ../usr/bin
for f in `cat %SOURCE4` ; do ln -s ../../bin/toybox $f ; done
cd ../../usr/sbin
for f in `cat %SOURCE5` ; do ln -s ../../bin/toybox $f ; done
popd

%files
%manifest toybox.manifest
%license LICENSE
%defattr(-,root,root,-)
/bin/toybox
/usr/bin/nslookup
%if "%{?profile}"=="tv"
/sbin/ping
/bin/ping
/sbin/ping6
/bin/ping6
%endif

%files symlinks-dhcp
%manifest toybox.manifest
%license LICENSE
%defattr(-,root,root,-)
%{_bindir}/dhcp

%files symlinks-dhcpd
%manifest toybox.manifest
%license LICENSE
%defattr(-,root,root,-)
%{_bindir}/dumpleases
%{_sbindir}/dhcpd
