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

#ifndef KNOWN_PLUGINS_H
#define KNOWN_PLUGINS_H

#include "plugincandidates.h"

#include <string>
#include <map>
#include <vector>

class KnownPlugins
{
    typedef std::vector<std::string> stringlist;
    
public:
    enum PluginType {
	VampPlugin,
	LADSPAPlugin,
	DSSIPlugin
    };

    KnownPlugins(std::string helperExecutableName,
                 PluginCandidates::LogCallback *cb = 0);

    std::vector<PluginType> getKnownPluginTypes() const {
	return { VampPlugin, LADSPAPlugin, DSSIPlugin };
    };
    
    std::string getTagFor(PluginType type) const {
	return m_known.at(type).tag;
    }

    stringlist getCandidateLibrariesFor(PluginType type) const {
	return m_candidates.getCandidateLibrariesFor(getTagFor(type));
    }

    std::string getFailureReport() const;
    
private:
    struct TypeRec {
	std::string tag;
	stringlist path;
	std::string descriptor;
    };
    std::map<PluginType, TypeRec> m_known;

    stringlist expandConventionalPath(PluginType type, std::string var);
    std::string getDefaultPath(PluginType type);

    PluginCandidates m_candidates;
};

#endif
