/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef PLUGIN_CANDIDATES_H
#define PLUGIN_CANDIDATES_H

#include <string>
#include <vector>
#include <map>

/**
 * Class to identify and list candidate shared-library files possibly
 * containing plugins. Uses a separate process (the "helper", whose
 * executable name must be provided at construction) to test-load each
 * library in order to winnow out any that fail to load or crash on
 * load.
 *
 * Requires C++11 and the Qt5 QtCore library.
 */
class PluginCandidates
{
    typedef std::vector<std::string> stringlist;
    
public:
    /** Construct a PluginCandidates scanner that uses the given
     *  executable as its load check helper.
     */
    PluginCandidates(std::string helperExecutableName);

    /** Scan the libraries found in the given plugin path (i.e. list
     *  of plugin directories), checking that the given descriptor
     *  symbol can be looked up in each. Store the results
     *  internally, associated with the given (arbitrary) tag, for
     *  later querying using getCandidateLibrariesFor() and
     *  getFailedLibrariesFor().
     *
     *  Not thread-safe.
     */
    void scan(std::string tag,
	      stringlist pluginPath,
	      std::string descriptorSymbolName);

    /** Return list of plugin library paths that were checked
     *  successfully during the scan for the given tag.
     */
    stringlist getCandidateLibrariesFor(std::string tag) const;

    struct FailureRec {
	std::string library;
	std::string message;
    };

    /** Return list of failure reports arising from the prior scan for
     *  the given tag. 
     */
    std::vector<FailureRec> getFailedLibrariesFor(std::string tag) const;

private:
    std::string m_helper;
    std::map<std::string, stringlist> m_candidates;
    std::map<std::string, std::vector<FailureRec> > m_failures;

    stringlist getLibrariesInPath(stringlist path);
    stringlist runHelper(stringlist libraries, std::string descriptor);
    void recordResult(std::string tag, stringlist results);
};

#endif
