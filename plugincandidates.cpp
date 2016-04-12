/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "plugincandidates.h"

#include <set>
#include <stdexcept>
#include <iostream>

#include <QProcess>
#include <QDir>

#ifdef _WIN32
#define PLUGIN_GLOB "*.dll"
#define PATH_SEPARATOR ';'
#else
#define PATH_SEPARATOR ':'
#ifdef __APPLE__
#define PLUGIN_GLOB "*.dylib *.so"
#else
#define PLUGIN_GLOB "*.so"
#endif
#endif

using namespace std;

PluginCandidates::PluginCandidates(string helperExecutableName) :
    m_helper(helperExecutableName)
{
}

vector<string>
PluginCandidates::getCandidateLibrariesFor(string tag)
{
    return m_candidates[tag];
}

vector<PluginCandidates::FailureRec>
PluginCandidates::getFailedLibrariesFor(string tag)
{
    return m_failures[tag];
}

vector<string>
PluginCandidates::getLibrariesInPath(vector<string> path)
{
    vector<string> candidates;

    for (string dirname: path) {

//#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
        cerr << "getLibrariesInPath: scanning directory " << dirname << endl;
//#endif

        QDir dir(dirname.c_str(), PLUGIN_GLOB,
		 QDir::Name | QDir::IgnoreCase,
		 QDir::Files | QDir::Readable);

	for (unsigned int i = 0; i < dir.count(); ++i) {
            QString soname = dir.filePath(dir[i]);
            candidates.push_back(soname.toStdString());
        }
    }

    return candidates;
}

void
PluginCandidates::scan(string tag,
		       vector<string> pluginPath,
		       string descriptorFunctionName)
{
    vector<string> libraries = getLibrariesInPath(pluginPath);
    vector<string> remaining = libraries;

    int runlimit = 20;
    int runcount = 0;
    
    vector<string> result;
    
    while (result.size() < libraries.size() && runcount < runlimit) {
	vector<string> output = runHelper(remaining, descriptorFunctionName);
	result.insert(result.end(), output.begin(), output.end());
	int shortfall = int(remaining.size()) - int(output.size());
	if (shortfall > 0) {
	    // Helper bailed out for some reason presumably associated
	    // with the plugin following the last one it reported
	    // on. Add a null entry for that one and continue with the
	    // following ones.
	    result.push_back("");
	    if (shortfall == 1) {
		remaining = vector<string>();
	    } else {
		remaining = vector<string>
		    (remaining.rbegin(), remaining.rbegin() + shortfall - 1);
	    }
	}
	++runcount;
    }

    recordResult(tag, result);
}

vector<string>
PluginCandidates::runHelper(vector<string> libraries, string descriptor)
{
    vector<string> output;
    cerr << "running helper with following library list:" << endl;
    for (auto &lib: libraries) cerr << lib << endl;

    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.start(m_helper.c_str(), { descriptor.c_str() });
    if (!process.waitForStarted()) {
	cerr << "helper failed to start" << endl;
	throw runtime_error("plugin load helper failed to start");
    }
    for (auto &lib: libraries) {
	process.write(lib.c_str(), lib.size());
	process.write("\n", 1);
    }

    int buflen = 4096;
    while (process.waitForReadyRead()) {
	char buf[buflen];
	qint64 linelen = process.readLine(buf, buflen);
//        cerr << "read line: " << buf;
	if (linelen < 0) {
	    cerr << "read failed from plugin load helper" << endl;
	    return output;
	}
	output.push_back(buf);
        if (output.size() == libraries.size()) {
            process.close();
            process.waitForFinished();
            break;
        }
    }	
	
    return output;
}

void
PluginCandidates::recordResult(string tag, vector<string> result)
{
    cerr << "recordResult: not yet implemented, but result was:" << endl;
    for (auto &r: result) cerr << r;
    cerr << "(ends)" << endl;
}

int main(int argc, char **argv)
{
    //!!! just a test
    PluginCandidates candidates("./helper");
    candidates.scan("vamp",
                    { "/usr/lib/vamp", "/usr/local/lib/vamp" },
                    "vampGetPluginDescriptor");
}

