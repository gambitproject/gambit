Summary: Gambit Game Theory Tools
Name: gambit
Version: 0.2006.01.20
Release: 1
License: GPL
Group: Applications
URL: http://econweb.tamu.edu/gambit
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
Gambit is a collection of tools for interactively building and
analyzing finite games.

%prep
%setup -q 
%configure

%build
make

%install
%makeinstall

mkdir -p $RPM_BUILD_ROOT%{_datadir}/gambit
cp -R efg $RPM_BUILD_ROOT%{_datadir}/gambit
cp -R nfg $RPM_BUILD_ROOT%{_datadir}/gambit 

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README
%{_bindir}/gambit*
%{_libdir}/libgambit*
%{_libdir}/liblab*
%{_includedir}/libgambit*
%{_datadir}/gambit


%changelog
* Tue Dec 13 2005 Ted Turocy <turocy@econmail.tamu.edu>
- adding to main distribution; tweak descriptions

* Mon Dec 12 2005 Paul E. Johnson <pauljohn@ku.edu>
- version bump & reorganization

* Tue Apr 06 2004 Paul E. Johnson <pauljohn@ku.edu>
- version bump.

* Wed Mar 10 2004 Christopher C. Weis <christopher-c-weis@uiowa.edu>
- cleaned up spec file to use macros and proper file inclusions

* Thu Oct 23 2003 Paul E. Johnson <pauljohn@pjdell.pols.ukans.edu> 
- Initial build.


