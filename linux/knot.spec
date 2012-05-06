Summary: Knot
Name: knot
Version: 0.1.1
Release: 1
Source0: %{name}-%{version}.tar.gz
License: MIT
Group: Productivity/Graphics/Visualization/Graph
BuildRoot: %{_builddir}/%{name}-root
%description
Knot is a package for visualising 3D Celtic Knots. The
knot models can also be exported for loading into 3D
modelling applications or 3D printing.
%prep
%setup -q
%build
./configure
make
%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
%clean
rm -rf $RPM_BUILD_ROOT
%files
%defattr(-,root,root)
/usr/local/bin/knot
/usr/local/share/knot/application.glade
%doc /usr/local/share/doc/knot/AUTHORS
%doc /usr/local/share/doc/knot/COPYING
%doc /usr/local/share/doc/knot/ChangeLog
%doc /usr/local/share/doc/knot/INSTALL
%doc /usr/local/share/doc/knot/NEWS
%doc /usr/local/share/doc/knot/README
%doc COPYING AUTHORS README NEWS INSTALL ChangeLog

