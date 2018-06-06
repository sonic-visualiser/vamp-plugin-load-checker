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

/**
 * Class to identify and list candidate shared-library files possibly
 * containing plugins in a hardcoded set of known formats. Uses a
 * separate process (the "helper", whose executable name must be
 * provided at construction) to test-load each library in order to
 * winnow out any that fail to load or crash on load.
 *
 * Requires C++11 and the Qt5 QtCore library.
 */
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

    std::vector<PluginType> getKnownPluginTypes() const;
    
    std::string getTagFor(PluginType type) const {
        return m_known.at(type).tag;
    }

    stringlist getCandidateLibrariesFor(PluginType type) const {
        return m_candidates.getCandidateLibrariesFor(getTagFor(type));
    }

    std::string getPathEnvironmentVariableFor(PluginType type) const {
        return m_known.at(type).variable;
    }
    
    stringlist getPathFor(PluginType type) const {
        return m_known.at(type).path;
    }

    std::string getHelperExecutableName() const {
        return m_helperExecutableName;
    }

    std::string getFailureReport() const;
    
private:
    struct TypeRec {
        std::string tag;
        std::string variable;
        stringlist path;
        std::string descriptor;
    };
    typedef std::map<PluginType, TypeRec> Known;
    Known m_known;

    stringlist expandConventionalPath(PluginType type, std::string var);
    std::string getDefaultPath(PluginType type);

    PluginCandidates m_candidates;
    std::string m_helperExecutableName;

    /** This returns true if the helper has a name ending in "-32". By
     *  our convention, this means that it is a 32-bit helper found on
     *  a 64-bit system, so (depending on the OS) we may need to look
     *  in 32-bit-specific paths. Note that is32bit() is *not* usually
     *  true on 32-bit systems; it's used specifically to indicate a
     *  "non-native" 32-bit helper.
     */
    bool is32bit() const;
};

#endif
