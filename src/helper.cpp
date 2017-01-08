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

/*
    Copyright (c) 2016 Queen Mary, University of London

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music and Queen Mary, University of London shall not be
    used in advertising or otherwise to promote the sale, use or other
    dealings in this Software without prior written authorization.
*/

#include "../version.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <string>
#ifdef UNICODE
static std::string lastLibraryName = "";
static HMODULE LoadLibraryUTF8(std::string name) {
    lastLibraryName = name;
    int n = name.size();
    int wn = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), n, 0, 0);
    wchar_t *wname = new wchar_t[wn+1];
    wn = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), n, wname, wn);
    wname[wn] = L'\0';
    HMODULE h = LoadLibraryW(wname);
    delete[] wname;
    return h;
}
static std::string GetErrorText() {
    wchar_t *buffer;
    DWORD err = GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &buffer,
        0, NULL );
    int wn = wcslen(buffer);
    int n = WideCharToMultiByte(CP_UTF8, 0, buffer, wn, 0, 0, 0, 0);
    if (n < 0) {
        LocalFree(&buffer);
        return "Unable to convert error string (internal error)";
    }
    char *text = new char[n+1];
    (void)WideCharToMultiByte(CP_UTF8, 0, buffer, wn, text, n, 0, 0);
    text[n] = '\0';
    std::string s(text);
    LocalFree(&buffer);
    delete[] text;
    for (int i = s.size(); i > 0; ) {
        --i;
        if (s[i] == '\n' || s[i] == '\r') {
            s.erase(i, 1);
        }
    }
    std::size_t pos = s.find("%1");
    if (pos != std::string::npos && lastLibraryName != "") {
        s.replace(pos, 2, lastLibraryName);
    }
    return s;
}
#define DLOPEN(a,b)  LoadLibraryUTF8(a)
#else
#define DLOPEN(a,b)  LoadLibrary((a).c_str())
#define GetErrorText() ""
#endif
#define DLSYM(a,b)   (void *)GetProcAddress((HINSTANCE)(a),(b).c_str())
#define DLCLOSE(a)   (!FreeLibrary((HINSTANCE)(a)))
#define DLERROR()    (GetErrorText())
#else
#include <dlfcn.h>
#define DLOPEN(a,b)  dlopen((a).c_str(),(b))
#define DLSYM(a,b)   dlsym((a),(b).c_str())
#define DLCLOSE(a)   dlclose((a))
#define DLERROR()    dlerror()
#endif

#include <string>
#include <iostream>

//#include <unistd.h>

using namespace std;

string error()
{
    string e = DLERROR();
    if (e == "") return "(unknown error)";
    else return e;
}

string checkLADSPAStyleDescriptorFn(void *f)
{
    typedef const void *(*DFn)(unsigned long);
    DFn fn = DFn(f);
    unsigned long index = 0;
    while (fn(index)) ++index;
    if (index == 0) return "Library contains no plugins";
//    else cerr << "Library contains " << index << " plugin(s)" << endl;
    return "";
}

string checkVampDescriptorFn(void *f)
{
    typedef const void *(*DFn)(unsigned int, unsigned int);
    DFn fn = DFn(f);
    unsigned int index = 0;
    while (fn(2, index)) ++index;
    if (index == 0) return "Library contains no plugins";
//    else cerr << "Library contains " << index << " plugin(s)" << endl;
    return "";
}

string check(string soname, string descriptor)
{
    void *handle = DLOPEN(soname, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        return "Unable to open plugin library: " + error();
    }

    void *fn = DLSYM(handle, descriptor);
    if (!fn) {
        return "Failed to find plugin descriptor " + descriptor +
            " in library: " + error();
    }

    if (descriptor == "ladspa_descriptor") {
        return checkLADSPAStyleDescriptorFn(fn);
    } else if (descriptor == "dssi_descriptor") {
        return checkLADSPAStyleDescriptorFn(fn);
    } else if (descriptor == "vampGetPluginDescriptor") {
        return checkVampDescriptorFn(fn);
    } else {
        cerr << "Note: no descriptor logic known for descriptor function \""
             << descriptor << "\"; not actually calling it" << endl;
    }
    
    return "";
}

int main(int argc, char **argv)
{
    bool allGood = true;
    string soname;

    bool showUsage = false;
    
    if (argc > 1) {
        string opt = argv[1];
        if (opt == "-?" || opt == "-h" || opt == "--help") {
            showUsage = true;
        } else if (opt == "-v" || opt == "--version") {
            cout << CHECKER_VERSION << endl;
            return 0;
        }
    } 
    
    if (argc != 2 || showUsage) {
        cerr << endl;
        cerr << "plugin-checker-helper: Test shared library objects for plugins to be" << endl;
        cerr << "loaded via descriptor functions." << endl;
        cerr << "\n    Usage: plugin-checker-helper <descriptorname>\n"
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
