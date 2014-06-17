# Initial spec file created by autospec ver. 0.6 with rpm 2.5 compatibility
Summary: .wav file normaliser, wav player, wav recorder
# The Summary: line should be expanded to about here -----^
Name: wavnorm
Version: 0.5
Release: 1
Group: unknown
Copyright: GPL
Source: wavnorm-%{version}.tar.gz
BuildRoot: /var/tmp/wavnorm-%{version}-root
# Following are optional fields
URL: http://www.linuxbandwagon.com/wavnorm/
Vendor: LinuxBandwagon and ZOG
#Distribution: Red Hat Contrib-Net
#Patch: wavnorm-%{version}.patch
Prefix: /usr/local
#BuildArchitectures: noarch
Requires: newt
#Obsoletes: 

%description
wavnorm version 0.5
wav file normaliser and analyser
nrecord wav recorder with VU meter
nplay wav player with VU meter

%prep
%setup
#%patch

%build
#CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr
make

%install

make install INSTALLDIR=$RPM_BUILD_ROOT

#%clean
#[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
/usr/local/bin/wavnorm
/usr/local/bin/nplay
/usr/local/bin/nrecord
%doc /usr/local/man/man1/wavnorm.1
%doc /usr/local/man/man1/nplay.1
%doc /usr/local/man/man1/nrecord.1
%doc COPYING
%doc Changelog
%doc README

%changelog
* Tue Jul 17 2001 root <root@teleport.zog.net.au>
- Initial spec file created by autospec ver. 0.6 with rpm 2.5 compatibility
