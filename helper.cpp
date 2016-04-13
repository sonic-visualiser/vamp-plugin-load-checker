/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/**
 * Plugin Load Checker Helper
 *
 * This program accepts the name of a descriptor symbol as its only
 * command-line argument. It then reads a list of plugin library paths
 * from stdin, one per line. For each path read, it attempts to load
 * that library and retrieve the named descriptor symbol, printing a
 * line to stdout reporting whether this was successful or not and
 * then flushing stdout. The output line format is described
 * below. The program exits with code 0 if all libraries were loaded
 * successfully and non-zero otherwise.
 *
 * Note that library paths must be ready to pass to dlopen() or
 * equivalent; this usually means they should be absolute paths.
 *
 * Output line for successful load of library libname.so:
 * SUCCESS|/path/to/libname.so|
 * 
 * Output line for failed load of library libname.so:
 * FAILURE|/path/to/libname.so|Reason for failure if available
 *
 * Sometimes plugins will crash completely on load, bringing down this
 * program with them. If the program exits before all listed plugins
 * have been checked, this means that the plugin following the last
 * reported one has crashed. Typically the caller may want to run it
 * again, omitting that plugin.
 */

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define DLOPEN(a,b)  LoadLibrary((a).toStdWString().c_str())
#define DLSYM(a,b)   GetProcAddress((HINSTANCE)(a),(b))
#define DLCLOSE(a)   (!FreeLibrary((HINSTANCE)(a)))
#define DLERROR()    ""
#else
#include <dlfcn.h>
#define DLOPEN(a,b)  dlopen((a).c_str(),(b))
#define DLSYM(a,b)   dlsym((a),(b).c_str())
#define DLCLOSE(a)   dlclose((a))
#define DLERROR()    dlerror()
#endif

#include <string>
#include <iostream>

using namespace std;

string error()
{
    string e = dlerror();
    if (e == "") return "(unknown error)";
    else return e;
}

string check(string soname, string descriptor)
{
//    cerr << "helper: trying: " << soname << endl;
    
    void *handle = DLOPEN(soname, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
//	cerr << "helper: failed to open" << endl;
	return "Unable to open plugin library: " + error();
    }

    void *fn = DLSYM(handle, descriptor);
    if (!fn) {
//	cerr << "helper: failed to find descriptor" << endl;
	return "Failed to find plugin descriptor " + descriptor +
	    " in library: " + error();
    }

//    cerr << "helper: succeeded" << endl;
    
    return "";
}

int main(int argc, char **argv)
{
    bool allGood = true;
    string soname;

    if (argc != 2) {
	cerr << "\nUsage:\n    " << argv[0] << " descriptorname\n"
	    "\nwhere descriptorname is the name of a plugin descriptor symbol to be sought\n"
	    "in each library (e.g. vampGetPluginDescriptor for Vamp plugins). The list of\n"
	    "candidate plugin library filenames is read from stdin.\n" << endl;
	return 2;
    }

    string descriptor = argv[1];
    
    while (getline(cin, soname)) {
	string report = check(soname, descriptor);
	if (report != "") {
	    cout << "FAILURE|" << soname << "|" << report << endl;
	    allGood = false;
	} else {
	    cout << "SUCCESS|" << soname << "|" << endl;
	}
    }

    return allGood ? 0 : 1;
}
