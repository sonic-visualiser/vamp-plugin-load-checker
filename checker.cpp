
#include "knownplugins.h"

#include <iostream>

using namespace std;

int main(int, char **)
{
    KnownPlugins kp;

    for (auto t: kp.getKnownPluginTypes()) {
	cout << "successful libraries for plugin type \""
	     << kp.getTagFor(t) << "\":" << endl;
	for (auto lib: kp.getCandidateLibrariesFor(t)) {
	    cout << lib << endl;
	}
    }

    cout << "Failure message (if any):" << endl;
    cout << kp.getFailureReport() << endl;
}

