
#include "plugincandidates.h"

int main(int argc, char **argv)
{
    //!!! just a test
    PluginCandidates candidates("./helper");
    candidates.scan("vamp",
                    { "/usr/lib/vamp", "/usr/local/lib/vamp" },
                    "vampGetPluginDescriptor");
}

