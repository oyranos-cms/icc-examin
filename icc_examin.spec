Summary: ICC profile viewer and colour visualisation
Name: icc_examin
Version: 0.32
Release: 1
License: GPL,LGPL
Group: Productivity/Graphics/Other
Source: icc_examin_0.32.tar.gz
#Source: http://www.behrmann.name/index.php?option=com_content&task=view&id=33&Itemid=69
URL: http://www.behrmann.name
Vendor: Kai-Uwe Behrmann
Packager: Kai-Uwe Behrmann <ku.b@gmx.de>
BuildRoot: %{_tmppath}/%{name}-root
#Requires: /sbin/ldconfig FIXME: Find dependencies
Prefix:    %{_prefix}

%description
ICC Examin is a small utility (unix name: icc_examin) for the
purpos of watching the internals of ICC profiles, measurement
data (CGATS), argylls gamut vrml visualisations and
video card gamma tables (Xorg/XFree86).


Authors:
--------
    ICC Examin  Kai-Uwe Behrmann <www.behrmann.name>

used components and libraries:
    agv         Philip Winston
    freeglut    Pawel W. Olszta
    littleCMS   Marti Maria
    FLTK        Bill Spizak and others
    FLU         Jason Bryan
    FTGL        Henry Maddocks
    Oyranos     Kai-Uwe Behrmann

%prep
%setup -q -n %{name}_%{version}
#-%{release}

%build
%configure
configure --prefix %{_prefix}
make %{_smp_mflags} static

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%clean
rm -rf %{buildroot}

%post

%postun

%files
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog README
%{_bindir}/icc_examin
%{_datadir}/applications/icc_examin.desktop
#%{_datadir}/pixmaps/icc_examin.png
%{_datadir}/fonts/FreeSans.ttf
%{_datadir}/locale/de/LC_MESSAGES/icc_examin.mo


%changelog
* Tue May 26 2005 Kai-Uwe Behrmann
- GUI improvements
- translation to german in v0.30

* Tue Apr 21 2005 Kai-Uwe Behrmann
- first release


