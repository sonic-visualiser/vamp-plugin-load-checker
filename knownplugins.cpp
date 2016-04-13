/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "knownplugins.h"

#include <sstream>

using namespace std;

#if defined(_WIN32)
#define PATH_SEPARATOR ';'
#else
#define PATH_SEPARATOR ':'
#endif

KnownPlugins::KnownPlugins() :
    m_candidates("./helper") //!!!??? wot to do
{
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
	    char *pfiles = getenv("ProgramFiles");
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

    int n = failures.size();
    int i = 0;

    ostringstream os;
    
    os << "<ul>";
    for (auto f: failures) {
	os << "<li>" + f.library;
	if (f.message != "") {
	    os << " (" + f.message + ")";
	} else {
	    os << " (unknown error)";
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
