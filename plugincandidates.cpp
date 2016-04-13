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

#include "plugincandidates.h"

#include <set>
#include <stdexcept>
#include <iostream>

#include <QProcess>
#include <QDir>

#if defined(_WIN32)
#define PLUGIN_GLOB "*.dll"
#elif defined(__APPLE__)
#define PLUGIN_GLOB "*.dylib *.so"
#else
#define PLUGIN_GLOB "*.so"
#endif

using namespace std;

PluginCandidates::PluginCandidates(string helperExecutableName) :
    m_helper(helperExecutableName)
{
}

vector<string>
PluginCandidates::getCandidateLibrariesFor(string tag) const
{
    if (m_candidates.find(tag) == m_candidates.end()) return {};
    else return m_candidates.at(tag);
}

vector<PluginCandidates::FailureRec>
PluginCandidates::getFailedLibrariesFor(string tag) const
{
    if (m_failures.find(tag) == m_failures.end()) return {};
    else return m_failures.at(tag);
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
		       string descriptorSymbolName)
{
    vector<string> libraries = getLibrariesInPath(pluginPath);
    vector<string> remaining = libraries;

    int runlimit = 20;
    int runcount = 0;
    
    vector<string> result;
    
    while (result.size() < libraries.size() && runcount < runlimit) {
	vector<string> output = runHelper(remaining, descriptorSymbolName);
	result.insert(result.end(), output.begin(), output.end());
	int shortfall = int(remaining.size()) - int(output.size());
	if (shortfall > 0) {
	    // Helper bailed out for some reason presumably associated
	    // with the plugin following the last one it reported
	    // on. Add a failure entry for that one and continue with
	    // the following ones.
            cerr << "shortfall = " << shortfall << " (of " << remaining.size() << ")" << endl;
	    result.push_back("FAILURE|" +
                             *(remaining.rbegin() + shortfall - 1) +
                             "|Plugin load check failed");
            remaining = vector<string>
                (remaining.rbegin(), remaining.rbegin() + shortfall - 1);
	}
	++runcount;
    }

    recordResult(tag, result);
}

vector<string>
PluginCandidates::runHelper(vector<string> libraries, string descriptor)
{
    vector<string> output;
//    cerr << "running helper with following library list:" << endl;
//    for (auto &lib: libraries) cerr << lib << endl;

    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setProcessChannelMode(QProcess::ForwardedErrorChannel);
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
    bool done = false;
    
    while (!done) {
	char buf[buflen];
	qint64 linelen = process.readLine(buf, buflen);
        if (linelen > 0) {
            output.push_back(buf);
            done = (output.size() == libraries.size());
        } else if (linelen < 0) {
            // error case
            done = true;
	} else {
            // no error, but no line read (could just be between
            // lines, or could be eof)
            done = (process.state() == QProcess::NotRunning);
            if (!done) process.waitForReadyRead(100);
        }
    }

    if (process.state() != QProcess::NotRunning) {
        process.close();
        process.waitForFinished();
    }
	
    return output;
}

void
PluginCandidates::recordResult(string tag, vector<string> result)
{
    for (auto &r: result) {

        QString s(r.c_str());
        QStringList bits = s.split("|");
        if (bits.size() < 2 || bits.size() > 3) {
            cerr << "Invalid helper output line: \"" << r << "\"" << endl;
            continue;
        }

        string status = bits[0].toStdString();
        
        string library = bits[1].toStdString();
        if (bits.size() == 2) library = bits[1].trimmed().toStdString();

        string message = "";
        if (bits.size() > 2) message = bits[2].trimmed().toStdString();
        
        if (status == "SUCCESS") {
            m_candidates[tag].push_back(library);

        } else if (status == "FAILURE") {
            m_failures[tag].push_back({ library, message });

        } else {
            cerr << "Unexpected status " << status
                 << " in helper output line: \"" << r << "\"" << endl;
        }
    }
}

