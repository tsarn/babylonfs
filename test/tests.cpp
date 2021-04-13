#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <thread>
#include <utility>
#include <fuse.h>
#include "doctest.h"

#include "../src/babylonfs.h"

#define seed "test_seed"
#define cycle -1
#define root "./test_babylonfs"

namespace fs = std::filesystem;

class BabylonFSKeeper {
private:
    std::string path;
    struct fuse_args args = FUSE_ARGS_INIT(0, {});
    const struct fuse_operations *ops = BabylonFS::run(seed, cycle);
    struct fuse_chan* chan;
    struct fuse* fuse;
    std::thread fuse_thread;

public:
    BabylonFSKeeper(std::string fs_path) : path(std::move(fs_path)) {
        fs::create_directory(path);

        chan = fuse_mount(path.c_str(), &args);
        fuse = fuse_new(chan, &args, ops, sizeof(*ops), nullptr);

        struct fuse *fuse_ptr = fuse;

        fuse_thread = std::thread([fuse_ptr]() { fuse_loop(fuse_ptr); });
    }

    BabylonFSKeeper(const BabylonFSKeeper&) = delete;
    BabylonFSKeeper& operator=(const BabylonFSKeeper&) = delete;

    ~BabylonFSKeeper() {
        fuse_exit(fuse);
        fuse_unmount(path.c_str(), chan);
        fuse_destroy(fuse);

        fuse_thread.join();

        fs::remove(path);
    }
};

void rooms_walk(const fs::path &path, int depth, int max_depth,
                const std::function<void(const fs::path&)>& checker) {
    if (depth > max_depth) {
        return;
    }
    checker(path);
    for (const auto& neighbour_path : fs::directory_iterator(path)) {
        if (neighbour_path.path().filename().string()[0] == 'k') {
            rooms_walk(neighbour_path, depth + 1, max_depth, checker);
        }
    }
}

void cupboards_walk(const fs::path &path, int depth, int max_depth,
                    const std::function<void(const fs::path&)>& checker) {
    rooms_walk(path, depth, max_depth, [&checker](const fs::path &path) {
        for (const auto& cupboard_name : {"b0", "b1", "b2", "b3"}) {
            auto cupboard_path = path;
            cupboard_path.append(cupboard_name);
            checker(cupboard_path);
        }
    });
}

void shelves_walk(const fs::path &path, int depth, int max_depth,
                  const std::function<void(const fs::path&)>& checker) {
    cupboards_walk(path, depth, max_depth, [&checker](const fs::path &path) {
        for (const auto& shelf_name : {"0", "1", "2", "3", "4"}) {
            auto shelf_path = path;
            shelf_path.append(shelf_name);
            checker(shelf_path);
        }
    });
}

void books_walk(const fs::path &path, int depth, int max_depth,
                  const std::function<void(const fs::path&)>& checker) {
    shelves_walk(path, depth, max_depth, [&checker](const fs::path &path) {
        for (const auto& book_path : fs::directory_iterator(path)) {
            checker(book_path);
        }
    });
}

TEST_CASE("Every room has 2 neighbour rooms") {
    BabylonFSKeeper keeper(root);

    rooms_walk(fs::path(root), 0, 10, [](const fs::path &path) {
        CHECK(fs::is_directory(path));
    });
}

TEST_CASE("\"Every room has 4 cupboards\"") {
    BabylonFSKeeper keeper(root);

    cupboards_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK(fs::is_directory(path));
    });
}

TEST_CASE("\"Cupboards cannot be removed\"") {
    BabylonFSKeeper keeper(root);

    cupboards_walk(fs::path(root), 0, 5, [](const fs::path &path) {
                CHECK_THROWS(fs::remove(path));
    });
}

TEST_CASE("\"Cupboards can be renamed\"") {
    BabylonFSKeeper keeper(root);

    cupboards_walk(fs::path(root), 0, 5, [](const fs::path &path) {
                CHECK_NOTHROW(fs::rename(path, path.string() + "n"));
    });
}

TEST_CASE("\"Every cupboard has 5 shelves\"") {
    BabylonFSKeeper keeper(root);

    cupboards_walk(fs::path(root), 0, 5, [](const fs::path &path) {
           int num_shelves = 0;
           for (const auto& s : fs::directory_iterator(path)) {
               num_shelves++;
           }
           CHECK(num_shelves == 5);
    });

    shelves_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK(fs::is_directory(path));
    });
}

TEST_CASE("\"Shelves cannot be removed\"") {
    BabylonFSKeeper keeper(root);

    shelves_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK_THROWS(fs::remove(path));
    });
}

TEST_CASE("\"Shelves can be renamed\"") {
    BabylonFSKeeper keeper(root);

    shelves_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK_NOTHROW(fs::rename(path, path.string() + "n"));
    });
}

TEST_CASE("\"Every shelf has 32 books\"") {
    BabylonFSKeeper keeper(root);

    shelves_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        int num_books = 0;
        for (const auto& s : fs::directory_iterator(path)) {
            num_books++;
        }
        CHECK(num_books == 32);
    });
}
