Summary		: A Development Environment for Qt4
Name		: qdevelop
Version		: 0.25
Release		: 1
License		: GPL
Group		: Development/C++
URL		: http://qdevelop.org/
Source		: %{name}-%{version}.tar.gz
Packager	: Diego Iastrubni <diegoiast@gmail.com>
BuildRoot	: %{_tmppath}/%{name}-%{version}-%{release}-builtroot
BuildRequires	: cmake libqt4-devel
Requires	: libqt4-devel ctags gdb

%description 
QDevelop is a development environment entirely dedicated to Qt4.
QDevelop requires Qt4, gcc under Linux or MinGW under Windows, possibly gdb for program debugging and ctags for code completion.
QDevelop is available in English, French, German, Dutch, Polish, Spanish, Chinese, Russian, Italian, Ukrainian, Czech and
Portuguese. If you want to translate in your language, please contact me.

QDevelop is not a Kdevelop like or reduced. It's an independent IDE dedicated to Qt and is totally independent of KDevelop. Less
complete, but faster, light and especially multi-platforms. QDevelop and KDevelop have different code sources.

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
* Sat Dec 8 2007 Diego Iastrubni <diegoiast@gmail.com> - 0.25-1
 - first package version
