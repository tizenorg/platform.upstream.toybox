Name: toybox
Version: 0.5.2
Release: 1%{?dist}
Summary: ToyBox Core utilities
Group: Base/Utilities
License: BSD-2-Clause-FreeBSD
URL: http://landley.net/toybox/about.html
Source: %{name}-%{version}.tar.bz2
Source1: config

BuildRequires : smack-devel
BuildRequires : libattr-devel
Requires : libattr

%description
Toybox combines the most common Linux command line utilities together into a single BSD-licensed executable

%prep
%setup -q

%build
cp %{SOURCE1} ./.config
make %{?_smp_mflags} toybox
mv toybox_unstripped toybox ;# tizen's build system (GBS) needs unstripped binaries

%install
output="build"
mkdir -p ./${output}
make PREFIX=./${output} install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_sbindir}
cp --preserve=links ./${output}/bin/* %{buildroot}%{_bindir}
cp --preserve=links ./${output}/sbin/* %{buildroot}%{_sbindir}
cp --preserve=links ./${output}/usr/bin/* %{buildroot}%{_bindir}
cp --preserve=links ./${output}/usr/sbin/* %{buildroot}%{_sbindir}

%files
%{_bindir}/*
%{_sbindir}/*
