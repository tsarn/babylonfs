#include <fuse.h>
#include <unistd.h>
#include <cstring>

#include "babylonfs.h"

void Directory::stat(struct stat *st) const {
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink = 2;
}

void File::stat(struct stat *st) const {
    st->st_mode = S_IFREG | 0644;
    st->st_nlink = 1;
    st->st_size = getContents().size();
}

const struct fuse_operations *BabylonFS::run(const char *seed) noexcept {
    auto &me = instance();
    if (seed == nullptr) {
        me.seed = ""; // TODO: random
    } else {
        me.seed = seed;
    }
    return reinterpret_cast<const fuse_operations *>(me.fuseOps.get());
}

Entity::ptr BabylonFS::getPath(const char *pathStr) const {
    std::filesystem::path path{pathStr};
    Entity::ptr cur = getRoot();
    for (const auto &element : path) {
        if (element == "/") {
            continue;
        }

        auto &dir = dynamic_cast<Directory &>(*cur);
        cur = dir.get(element);

        if (!cur) {
            throw std::exception{};
        }
    }

    return cur;
}

BabylonFS::BabylonFS() : fuseOps(std::make_unique<struct fuse_operations>()) {
    fuseOps->init = [](struct fuse_conn_info *conn) -> void * {
        (void) conn;
        return nullptr;
    };

    fuseOps->getattr = [](const char *path, struct stat *st) -> int {
        st->st_uid = getuid();
        st->st_gid = getgid();
        st->st_atime = time(nullptr);
        st->st_mtime = time(nullptr);

        try {
            instance().getPath(path)->stat(st);
        } catch (std::exception &e) {
            return -ENOENT;
        }

        return 0;
    };

    fuseOps->readdir = [](const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                          struct fuse_file_info *fi) -> int {
        (void) offset;
        (void) fi;

        try {
            auto entry = instance().getPath(path);
            auto &dir = dynamic_cast<Directory &>(*entry);

            filler(buf, ".", nullptr, 0);
            filler(buf, "..", nullptr, 0);
            for (const auto &name : dir.getContents()) {
                filler(buf, name.c_str(), nullptr, 0);
            }
        } catch (std::exception &e) {
            return -ENOENT;
        }

        return 0;
    };

    fuseOps->open = [](const char *path, struct fuse_file_info *fi) -> int {
        try {
            auto entry = instance().getPath(path);
            (void) dynamic_cast<File &>(*entry); // check that it's a file, sorry

            if ((fi->flags & O_ACCMODE) != O_RDONLY && dynamic_cast<Note *>(entry.get()) == nullptr)
                return -EACCES;
            //TODO create file if flags == |?| + check if it's possible (same like mkdir)
        } catch (std::exception &e) {
            return -ENOENT;
        }

        return 0;
    };

    fuseOps->create = [](const char *path, mode_t mode,
                         struct fuse_file_info *fi) -> int {
        int res;

        res = open(path, fi->flags, mode);
        if (res == -1)
            return -errno;

        fi->fh = res;
        return 0;
    };

    fuseOps->rename = [](const char *from, const char *to) -> int {
        try {
            auto entry = instance().getPath(from);

            entry->rename(to);
        } catch (std::exception &e) {
            return -errno;
        }
        return 0;
    };


    fuseOps->read = [](const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) -> int {
        // removes unused fi warning
        (void) fi;

        try {
            auto entry = instance().getPath(path);
            auto &file = dynamic_cast<File &>(*entry);
            auto len = file.getContents().size();

            if (std::cmp_less(offset, len)) {
                if (std::cmp_less(len, offset + size)) {
                    size = len - offset;
                }
                std::memcpy(buf, file.getContents().data() + offset, size);
            } else {
                size = 0;
            }
        } catch (std::exception &e) {
            return -ENOENT;
        }

        return size;
    };

    fuseOps->write = [](const char *path, const char *buf, size_t size, off_t offset,
                        struct fuse_file_info *fi) -> int {
        (void) fi;

        try {
            auto entry = instance().getPath(path);
            auto &file = dynamic_cast<File &>(*entry);
            auto len = file.getContents().size();

            file.write(buf, size, offset);
        } catch (std::exception &e) {
            return -ENOENT;
        }

        return size;
    };

    fuseOps->mkdir = [](const char *path, mode_t mode) -> int {
        try {
            auto path_path = std::filesystem::path(path);
            auto parent = path_path.parent_path();
            auto name = path_path.filename();
            auto entry = instance().getPath(parent.c_str());
            auto &dir = dynamic_cast<Directory &>(*entry);
            dir.mkdir(name);
        } catch (std::exception &e) {
            return -EPERM;
        }
    };
}

BabylonFS &BabylonFS::instance() noexcept {
    static BabylonFS singleton;
    return singleton;
}
