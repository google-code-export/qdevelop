Summary		: A Development Environment for Qt4
Name		: qdevelop
Version		: 0.24
Release		: 1
License		: GPL
Group		: Development/C++
URL		: http://qdevelop.org/
Source		: %{name}-%{version}.tar.gz
Packager	: Diego Iastrubni <elcuco@kde.org>
BuildRoot	: %{_tmppath}/%{name}-%{version}-%{release}-builtroot
BuildRequires	: cmake libqt4-devel
Requires	: libqt4-devel ctags gdb

%description 
QDevelop is a development environment entirely dedicated to Qt4.

%prep
%setup
mkdir cbuild
cd cbuild; cmake ../

%build
make -C cbuild

%install
[ "%{buildroot}" != '/' ] && rm -rf %{buildroot}
#mkdir -p $RPM_BUILD_ROOT/%{instaldir_bin}
#cp cmake/QDevelop $RPM_BUILD_ROOT/%{instaldir_bin}

mkdir -p %{buildroot}/usr/bin
cp cbuild/QDevelop %{buildroot}/usr/bin/qdevelop

%clean
rm -fr cbuild

%post
# empty

%preun
# empty

%postun
# empty

%files 
%defattr(-,root,root)
%doc copying
%doc ChangeLog.txt
%doc README.txt
/usr/bin/*
%{instaldir_bin}/*

%changelog
* Wed Jun 6 2007 Diego Iastrubni <diego.iastrubni@xorcom.com> - 0.24-1
 - first package version
