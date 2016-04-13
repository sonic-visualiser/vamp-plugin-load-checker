/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

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

    KnownPlugins();

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
