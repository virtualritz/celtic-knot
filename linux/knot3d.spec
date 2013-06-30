Summary: Knot3D
Name: knot3d
Version: 0.1.4
Release: 1
Source0: %{name}-%{version}.tar.gz
License: MIT
Group: Productivity/Graphics/Visualization/Graph
BuildRoot: %{_builddir}/%{name}-root
%description
Knot3D is a package for visualising 3D Celtic Knots. The
knot3d models can also be exported for loading into 3D
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
/usr/local/bin/knot3d
/usr/local/share/knot3d/application.glade
%doc /usr/local/share/doc/knot3d/AUTHORS
%doc /usr/local/share/doc/knot3d/COPYING
%doc /usr/local/share/doc/knot3d/ChangeLog
%doc /usr/local/share/doc/knot3d/INSTALL
%doc /usr/local/share/doc/knot3d/NEWS
%doc /usr/local/share/doc/knot3d/README
%doc COPYING AUTHORS README NEWS INSTALL ChangeLog

