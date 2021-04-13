#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <thread>
#include <utility>
#include <fuse.h>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include "doctest.h"

#include "../src/babylonfs.h"

#define seed "test_seed"
#define cycle 5
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

void desks_walk(const fs::path &path, int depth, int max_depth,
                  const std::function<void(const fs::path&)>& checker) {
    rooms_walk(path, depth, max_depth, [&checker](const fs::path &path) {
        auto desk_path = path;
        desk_path.append("desk");
        checker(desk_path);
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

TEST_CASE("Every room has 4 cupboards") {
    BabylonFSKeeper keeper(root);

    cupboards_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK(fs::is_directory(path));
    });
}

TEST_CASE("Cupboards cannot be removed") {
    BabylonFSKeeper keeper(root);

    cupboards_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK_THROWS(fs::remove(path));
    });
}

TEST_CASE("Cupboards can be renamed") {
    BabylonFSKeeper keeper(root);

    cupboards_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK_NOTHROW(fs::rename(path, path.string() + "n"));
    });
}

TEST_CASE("Every cupboard has 5 shelves") {
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

TEST_CASE("Shelves cannot be removed") {
    BabylonFSKeeper keeper(root);

    shelves_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK_THROWS(fs::remove(path));
    });
}

TEST_CASE("Shelves can be renamed") {
    BabylonFSKeeper keeper(root);

    shelves_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        CHECK_NOTHROW(fs::rename(path, path.string() + "n"));
    });
}

TEST_CASE("Every shelf has 32 books") {
    BabylonFSKeeper keeper(root);

    shelves_walk(fs::path(root), 0, 5, [](const fs::path &path) {
        int num_books = 0;
        for (const auto& s : fs::directory_iterator(path)) {
            num_books++;
        }
        CHECK(num_books == 32);
    });
}

TEST_CASE("Books naming") {
    BabylonFSKeeper keeper(root);

    books_walk(fs::path(root), 0, 1, [](const fs::path &path) {
        CHECK(std::regex_match(path.filename().string(), std::regex("[a-z.,]+")));
    });
}

TEST_CASE("Book sizes") {
    BabylonFSKeeper keeper(root);

    books_walk(fs::path(root), 0, 1, [](const fs::path &path) {
        CHECK(fs::file_size(path) == 4096 * 256);
    });
}

TEST_CASE("Book permissions") {
    BabylonFSKeeper keeper(root);

    books_walk(fs::path(root), 0, 1, [](const fs::path &path) {
        CHECK((fs::status(path).permissions() & fs::perms::others_read) == fs::perms::others_read);
        CHECK((fs::status(path).permissions() & fs::perms::others_write) == fs::perms::none);
    });
}

TEST_CASE("Every room has desk") {
    BabylonFSKeeper keeper(root);
    desks_walk(fs::path(root), 0, 7, [](const fs::path &path) {
        CHECK(fs::is_directory(path));
    });
}

TEST_CASE("Can create and remove files and directories on desk") {
    BabylonFSKeeper keeper(root);
    desks_walk(fs::path(root), 0, 0, [](const fs::path &path) {
        auto tmp_dir_path = path;
        tmp_dir_path.append("tmp_dir");
        auto file1_path = path;
        file1_path.append("file1");
        auto file2_path = tmp_dir_path;
        file1_path.append("file2");
        CHECK_NOTHROW(fs::create_directory(tmp_dir_path));
        CHECK_NOTHROW(std::ofstream f1(file1_path));
        CHECK_NOTHROW(std::ofstream f2(file2_path));
        CHECK_NOTHROW(fs::remove(file2_path));
        CHECK_NOTHROW(fs::remove(file1_path));
        CHECK_NOTHROW(fs::remove(tmp_dir_path));
    });
}

TEST_CASE("Can create and remove files and directories on desk") {
    BabylonFSKeeper keeper(root);
    desks_walk(fs::path(root), 0, 0, [](const fs::path &path) {
        auto tmp_dir_path = path;
        tmp_dir_path.append("tmp_dir");
        auto file1_path = path;
        file1_path.append("file1");
        auto file2_path = tmp_dir_path;
        file1_path.append("file2");
                CHECK_NOTHROW(fs::create_directory(tmp_dir_path));
                CHECK_NOTHROW(std::ofstream f1(file1_path));
                CHECK_NOTHROW(std::ofstream f2(file2_path));
                CHECK_NOTHROW(fs::remove(file2_path));
                CHECK_NOTHROW(fs::remove(file1_path));
                CHECK_NOTHROW(fs::remove(tmp_dir_path));
    });
}

TEST_CASE("Cannot create recursive subfolders on desk") {
    BabylonFSKeeper keeper(root);
    desks_walk(fs::path(root), 0, 0, [](const fs::path &path) {
        auto tmp_dir_path = path;
        tmp_dir_path.append("tmp_dir");
        auto tmp_dir2_path = tmp_dir_path;
        tmp_dir2_path.append("tmp_dir");
        CHECK_NOTHROW(fs::create_directory(tmp_dir_path));
        CHECK_THROWS(fs::create_directory(tmp_dir2_path));
    });
}

TEST_CASE("Can move books to desk and back") {
    BabylonFSKeeper keeper(root);
    books_walk(fs::path(root), 0, 0, [](const fs::path &path) {
        auto on_desk_path = path.parent_path().parent_path().parent_path()
                .append("desk").append(path.filename().string());
        CHECK_NOTHROW(fs::rename(path, on_desk_path));
        CHECK_NOTHROW(fs::rename(on_desk_path, path));
    });
}

TEST_CASE("Can't move books to another place") {
    BabylonFSKeeper keeper(root);
    books_walk(fs::path(root), 0, 0, [](const fs::path &path) {
        auto on_desk_path = path.parent_path().parent_path().parent_path()
                .append("desk").append(path.filename().string());
        auto shelf_path = path.parent_path();
        auto cupboard_path = shelf_path.parent_path();
        auto another_path = cupboard_path;
        if (shelf_path.filename().string() == "0") {
            another_path.append("1");
        } else {
            another_path.append("0");
        }
        another_path.append("_nm");
        CHECK_NOTHROW(fs::rename(path, on_desk_path));
        CHECK_THROWS(fs::rename(on_desk_path, another_path));
        CHECK_NOTHROW(fs::rename(on_desk_path, path));
    });
}

TEST_CASE("Cyclic library") {
    BabylonFSKeeper keeper(root);

    std::unordered_map<std::string, std::unordered_set<std::string>> room_books;

    rooms_walk(fs::path(root), 0, 10, [&room_books](const fs::path &path) {
        auto first_shelf_path = path;
        first_shelf_path.append("b0").append("0");

        std::unordered_set<std::string> books;

        for (const auto& book_path : fs::directory_iterator(first_shelf_path)) {
            books.insert(book_path.path().filename().string());
        }

        if (room_books.contains(path.filename())) {
            CHECK(room_books[path.filename().string()] == books);
        } else {
            room_books[path.filename().string()] = books;
        }
    });

    CHECK(room_books.size() == 5 + 1); // also root directory
}
