Name:		vice
Version:	2.2
Release:	1%{?dist}
Summary:	Versatile Commodore 8-bit Emulator

Group:		Amusements/Games
License:	GPLv2
URL:		http://www.viceteam.org/
Source0:	http://www.zimmers.net/anonftp/pub/cbm/crossplatform/emulators/VICE/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	libXpm-devel, libXaw-devel, Xaw3d-devel, gtk2-devel, ncurses-devel, ncurses-libs, gettext
Requires:	Xaw3d, ncurses
Prereq:         /sbin/ldconfig /sbin/install-info


%description
This is version 2.0 of VICE, the multi-platform C64, C128, VIC20,
PET, PLUS4 and CBM-II emulator.  This version can be compiled for
MSDOS, Win32, RiscOS, OS/2, BeOS, QNX 4.x, QNX 6.x, AmigaOS, GP2X
and for most Unix systems provided with the X Window System version 11, R5 or
later.


%prep
%setup -q


%build
%configure --docdir=%{_docdir}/%{name}-%{version}/doc --enable-gnomeui --enable-fullscreen
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
# Generated Makefile ignores docdir from configure
make install DESTDIR=%{buildroot} realdocdir=%{_docdir}/%{name}-%{version}/doc
rm %{buildroot}%{_infodir}/dir
%{__install} -D -p -m 0644 AUTHORS ChangeLog FEEDBACK COPYING INSTALL NEWS README %{buildroot}%{_docdir}/%{name}-%{version}
%find_lang vice


%clean
rm -rf %{buildroot}


%post
/sbin/install-info %{_infodir}/vice.info.gz %{_infodir}/dir


%preun
if [ "$1" = 0 ]; then
 /sbin/install-info --delete %{_infodir}/vice.info.gz %{_infodir}/dir
fi


%files -f vice.lang
%defattr(-,root,root,-)
%{_docdir}/%{name}-%{version}/*
%{_bindir}/*
%{_libdir}/vice/*
%{_infodir}/vice.info.gz
%{_mandir}/man1/* 


%changelog
* Tue Oct 28 2008 Ingvar Hagelund <ingvar@linpro.no> - 2.0-1
- Wrap up a new package
