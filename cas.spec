Summary: 	C++ Application Server
Name: 		cas
Version: 	3.8.0
Release: 	0%{?dist}
License: 	BSD
Source: 	cas-%{version}.tar.gz
Group:		System Environment/Libraries
BuildRoot: 	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	cmake gcc-c++ httpd-devel libcurl-devel expat-devel pcre-devel ctpp2-devel

%package mod_cas
Summary:	CAS loadable module for Apache 2.X HTTP server
Group:		System Environment/Libraries
Requires:	%{name} = %{version}-%{release}

%package cli
Summary:	CAS Command-line utilities
Group:		System Environment/Libraries
Requires:	%{name} = %{version}-%{release}

%package modules
Summary:	CAS loadable modules and plugins
Group:		System Environment/Libraries
Requires:	%{name} = %{version}-%{release}

%package devel
Summary:	Header files and development documentation for %{name}
Group: 		Development/Libraries
Requires:	%{name} = %{version}-%{release}

%description
C++ Application server
%description mod_cas
CAS loadable module for Apache 2.X HTTP server
%description cli
CAS Command-line utilities
%description modules
CAS loadable modules and plugin
%description devel
C++ Application server header files

This package contains the header files, static libraries and development
documentation for %{name}. If you like to develop programs using %{name},
you will need to install %{name}-devel.

%prep
%setup -q -n cas-%{version}

%build
cmake -D SKIP_RELINK_RPATH=ON . -DCMAKE_INSTALL_PREFIX=/usr -DCFLAGS="${CFLAGS}" -DCXXFLAGS="${CXXFLAGS}"
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir %{buildroot}
make DESTDIR=%{buildroot} install
%if %_lib == "lib64"
mkdir -p %{buildroot}/usr/lib64
mv %{buildroot}/usr/lib/* %{buildroot}/usr/lib64
%endif

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_libdir}/libcas.so*
%{_datadir}/cas/conf/global-config.xml.sample
%{_datadir}/cas/conf/vhost-config.xml.sample
%{_datadir}/cas/tmpl/redirect/html-redirect.tmpl
%{_datadir}/cas/tmpl/redirect/wml-redirect.tmpl

%files cli
%defattr(-,root,root,-)
%{_bindir}/cas-config
%{_bindir}/cas-globalconf
%{_bindir}/cas-hostconf
%{_bindir}/cas-regexp
%{_bindir}/cas-server
%{_bindir}/cas-xmlrpc-parser

%files mod_cas
%{_libdir}/httpd/modules/mod_cas2.so

%files modules
%{_libdir}/cas/mod_ctpp_view.so
%{_libdir}/cas/mod_http_redirect_view.so
%{_libdir}/cas/mod_json_view.so
%{_libdir}/cas/mod_jsonrpc_view.so
%{_libdir}/cas/mod_plain_view.so
%{_libdir}/cas/mod_tabseparated_view.so
%{_libdir}/cas/mod_xmlrpc_view.so
%{_libdir}/cas/mod_xml_view.so
%{_libdir}/cas/mod_bson_view.so
%{_libdir}/cas/mod_bsonrpc_view.so

%files devel
%defattr(-,root,root,-)
%{_bindir}/cas-xt
%{_includedir}/cas
%{_libdir}/libcas.a
%{_libdir}/cas/mod_example_handler.so
%{_libdir}/cas/mod_example_module.so
%{_libdir}/cas/mod_example_view.so
%{_libdir}/libexampleclass.so*
%{_datadir}/cas/examples/request.json
%{_datadir}/cas/xt/CMakeLists.tmpl
%{_datadir}/cas/xt/ConfigureLibrary.cmake
%{_datadir}/cas/xt/ConfigurePackage.cmake
%{_datadir}/cas/xt/HandlerTemplate.tmpl
%{_datadir}/cas/xt/PreRequestHandlerTemplate.tmpl
%{_datadir}/cas/xt/RenameModule.cmake
%{_datadir}/cas/xt/ViewTemplate.tmpl

%changelog
* Mon Jan 11 2016 Andrey V. Shetukhin <stellar@communico.pro> - 3.7.4-0
- XML Parser bug fixes

* Mon Oct  5 2015 Andrey V. Shetukhin <stellar@communico.pro> - 3.7.3-0
- Upload module bug fix

* Tue Jun  2 2014 Andrey V. Shetukhin <stellar@communico.pro> - 3.7.2-0
- Version 3.7.2 release.
