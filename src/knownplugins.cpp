/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
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

#include "knownplugins.h"

#include <sstream>

using namespace std;

#if defined(_WIN32)
#define PATH_SEPARATOR ';'
#else
#define PATH_SEPARATOR ':'
#endif

KnownPlugins::KnownPlugins(string helperExecutableName,
                           PluginCandidates::LogCallback *cb) :
    m_candidates(helperExecutableName)
{
    m_candidates.setLogCallback(cb);
    
    m_known = {
	{
	    VampPlugin,
	    {
		"vamp",
		expandConventionalPath(VampPlugin, "VAMP_PATH"),
		"vampGetPluginDescriptor"
	    },
	}, {
	    LADSPAPlugin,
	    {
		"ladspa",
		expandConventionalPath(LADSPAPlugin, "LADSPA_PATH"),
		"ladspa_descriptor"
	    },
	}, {
	    DSSIPlugin,
	    {
		"dssi",
		expandConventionalPath(DSSIPlugin, "DSSI_PATH"),
		"dssi_descriptor"
	    }
	}
    };

    for (const auto &k: m_known) {
	m_candidates.scan(k.second.tag, k.second.path, k.second.descriptor);
    }
}

string
KnownPlugins::getDefaultPath(PluginType type)
{
    switch (type) {

#if defined(_WIN32)

    case VampPlugin:
	return "%ProgramFiles%\\Vamp Plugins";
    case LADSPAPlugin:
	return "%ProgramFiles%\\LADSPA Plugins;%ProgramFiles%\\Audacity\\Plug-Ins";
    case DSSIPlugin:
	return "%ProgramFiles%\\DSSI Plugins";
	
#elif defined(__APPLE__)
	
    case VampPlugin:
	return "$HOME/Library/Audio/Plug-Ins/Vamp:/Library/Audio/Plug-Ins/Vamp";
    case LADSPAPlugin:
	return "$HOME/Library/Audio/Plug-Ins/LADSPA:/Library/Audio/Plug-Ins/LADSPA";
    case DSSIPlugin:
	return "$HOME/Library/Audio/Plug-Ins/DSSI:/Library/Audio/Plug-Ins/DSSI";
	
#else /* Linux, BSDs, etc */
	
    case VampPlugin:
	return "$HOME/vamp:$HOME/.vamp:/usr/local/lib/vamp:/usr/lib/vamp";
    case LADSPAPlugin:
	return "$HOME/ladspa:$HOME/.ladspa:/usr/local/lib/ladspa:/usr/lib/ladspa";
    case DSSIPlugin:
	return "$HOME/dssi:$HOME/.dssi:/usr/local/lib/dssi:/usr/lib/dssi";
#endif
    }

    throw logic_error("unknown or unhandled plugin type");
}

vector<string>
KnownPlugins::expandConventionalPath(PluginType type, string var)
{
    vector<string> pathList;
    string path;

    char *cpath = getenv(var.c_str());
    if (cpath) path = cpath;

    if (path == "") {

        path = getDefaultPath(type);

	if (path != "") {

	    char *home = getenv("HOME");
	    if (home) {
		string::size_type f;
		while ((f = path.find("$HOME")) != string::npos &&
		       f < path.length()) {
		    path.replace(f, 5, home);
		}
	    }

#ifdef _WIN32
        const char *pfiles = getenv("ProgramFiles");
	    if (!pfiles) pfiles = "C:\\Program Files";
	    {
		string::size_type f;
		while ((f = path.find("%ProgramFiles%")) != string::npos &&
		       f < path.length()) {
		    path.replace(f, 14, pfiles);
		}
	    }
#endif
	}
    }

    string::size_type index = 0, newindex = 0;

    while ((newindex = path.find(PATH_SEPARATOR, index)) < path.size()) {
	pathList.push_back(path.substr(index, newindex - index).c_str());
	index = newindex + 1;
    }
    
    pathList.push_back(path.substr(index));

    return pathList;
}

string
KnownPlugins::getFailureReport() const
{
    vector<PluginCandidates::FailureRec> failures;

    for (auto t: getKnownPluginTypes()) {
	auto ff = m_candidates.getFailedLibrariesFor(getTagFor(t));
	failures.insert(failures.end(), ff.begin(), ff.end());
    }

    if (failures.empty()) return "";

    int n = int(failures.size());
    int i = 0;

    ostringstream os;
    
    os << "<ul>";
    for (auto f: failures) {
	os << "<li>" + f.library;
	if (f.message != "") {
	    os << "<br><i>" + f.message + "</i>";
	} else {
	    os << "<br><i>unknown error</i>";
	}
	os << "</li>";

	if (n > 10) {
	    if (++i == 5) {
		os << "<li>(... and " << (n - i) << " further failures)</li>";
		break;
	    }
	}
    }
    os << "</ul>";

    return os.str();
}
