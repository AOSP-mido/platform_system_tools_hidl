#include "AST.h"
#include "Coordinator.h"
#include "FQName.h"

#include <android-base/logging.h>
#include <stdio.h>
#include <unistd.h>

using namespace android;

static void usage(const char *me) {
    fprintf(stderr, "usage: %s -o output-path fqname ...\n", me);
}

int main(int argc, char **argv) {
    std::string outputDir;

    const char *me = argv[0];

    int res;
    while ((res = getopt(argc, argv, "ho:")) >= 0) {
        switch (res) {
            case 'o':
            {
                outputDir = optarg;
                break;
            }

            case '?':
            case 'h':
            default:
            {
                usage(me);
                exit(1);
                break;
            }
        }
    }

    argc -= optind;
    argv += optind;

    // Valid options are now in argv[0] .. argv[argc - 1].

    if (outputDir.empty()) {
        usage(me);
        exit(1);
    } else {
        const size_t len = outputDir.size();
        if (outputDir[len - 1] != '/') {
            outputDir += "/";
        }
    }

    const char *TOP = getenv("TOP");
    if (TOP == NULL) {
        LOG(ERROR) << "Your environment does not define $TOP.";
        return 1;
    }

    std::string interfacesPath = TOP;
    interfacesPath.append("/hardware/interfaces/");

    Coordinator coordinator(interfacesPath);

    for (int i = 0; i < argc; ++i) {
        FQName fqName(argv[i]);
        CHECK(fqName.isValid() && fqName.isFullyQualified());

        AST *ast = coordinator.parse(fqName);
        CHECK(ast != NULL);

        ast->generateCpp(outputDir);

        delete ast;
        ast = NULL;
    }

    return 0;
}
