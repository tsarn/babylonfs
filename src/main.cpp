#include "babylonfs.h"
#include <cstddef>
#include <iostream>
#include <fuse.h>

struct Options {
    const char* seed = nullptr;
    bool showHelp = false;
    int cycle = -1;
};

#define OPTION(t, p) { t, offsetof(Options, p), 1 }

static const struct fuse_opt optionsSpec[] = {
    OPTION("--seed=%s", seed),
    OPTION("--cycle=%d", cycle),
    OPTION("-h", showHelp),
    OPTION("--help", showHelp),
    FUSE_OPT_END
};

int main(int argc, char **argv) {
    Options options;

    if (argc == 1) {
        options.showHelp = true;
    }

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    if (fuse_opt_parse(&args, &options, optionsSpec, nullptr) == -1) {
        return 1;
    }

    if (options.showHelp) {
        fuse_opt_add_arg(&args, "--help");
        std::cout << R"(BabylonFS specific options:
    --seed=SEED         Seed for the random generator
    --cycle=CYCLE       Walk in circles!

)";
    }

    int exitCode = fuse_main(args.argc, args.argv, BabylonFS::run(options.seed, options.cycle), nullptr);
    fuse_opt_free_args(&args);
    return exitCode;
}
