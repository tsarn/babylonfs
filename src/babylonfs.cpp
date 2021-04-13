#include <fuse.h>
#include <unistd.h>
#include <cstring>
#include <utility>
#include <system_error>
#include <iostream>

#include "babylonfs.h"

void throwError(std::errc code) {
    throw std::system_error{std::make_error_code(code)};
}

void Entity::move(Entity &, const std::string&) {
    throwError(std::errc::permission_denied);
}

void File::write(const char *, size_t, off_t) {
    throwError(std::errc::permission_denied);
}

int File::getSize() {
    return getContents().size();
}

bool File::isWriteable() {
    return false;
}

void Directory::createFile(const std::string &) {
    throwError(std::errc::permission_denied);
}

void Directory::createDirectory(const std::string &) {
    throwError(std::errc::permission_denied);
}

void Directory::stat(struct stat *st) {
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink = 2;
}

void Directory::deleteDirectory(const std::string &) {
    throwError(std::errc::permission_denied);
}

void Directory::deleteFile(const std::string &) {
    throwError(std::errc::permission_denied);
}

void File::stat(struct stat *st) {
    st->st_mode = S_IFREG | 0644;
    st->st_nlink = 1;
    st->st_size = getSize();
}

const struct fuse_operations *BabylonFS::run(const char *seed, int cycle) noexcept {
    auto &me = instance();
    if (seed == nullptr) {
        me.seed = "";
    } else {
        me.seed = seed;
    }
    me.cycle = cycle;
    return me.fuseOps.get();
}

Entity::ptr BabylonFS::getPath(const std::string& pathStr) {
    std::filesystem::path path{pathStr};
    Entity::ptr cur = getRoot();
    for (const auto &element : path) {
        if (element == "/") {
            continue;
        }

        auto *dir = dynamic_cast<Directory*>(cur.get());
        cur = dir ? dir->get(element) : nullptr;

        if (!cur) {
            throwError(std::errc::no_such_file_or_directory);
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
        } catch (std::system_error &e) {
            return -e.code().value();
        }

        return 0;
    };

    fuseOps->readdir = [](const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                          struct fuse_file_info *fi) -> int {
        (void) offset;
        (void) fi;

        try {
            auto entity = instance().getPath(path);
            auto *dir = dynamic_cast<Directory*>(entity.get());

            if (!dir) {
                throwError(std::errc::not_a_directory);
            }

            filler(buf, ".", nullptr, 0);
            filler(buf, "..", nullptr, 0);
            for (const auto &name : dir->getContents()) {
                filler(buf, name.c_str(), nullptr, 0);
            }
        } catch (std::system_error &e) {
            return -e.code().value();
        }

        return 0;
    };

    fuseOps->open = [](const char *path, struct fuse_file_info *fi) -> int {
        try {
            auto entity = instance().getPath(path);
            auto* file = dynamic_cast<File*>(entity.get());

            if (!file) {
                throwError(std::errc::is_a_directory);
            }

            if ((fi->flags & O_ACCMODE) != O_RDONLY && !file->isWriteable()) {
                throwError(std::errc::permission_denied);
            }
        } catch (std::system_error &e) {
            return -e.code().value();
        }

        return 0;
    };

    fuseOps->create = [](const char *pathStr, mode_t mode,
                         struct fuse_file_info *fi) -> int {
        (void)mode;
        (void)fi;

        std::filesystem::path path{pathStr};
        try {
            auto entity = instance().getPath(path.parent_path());
            auto *dir = dynamic_cast<Directory*>(entity.get());

            if (!dir) {
                throwError(std::errc::not_a_directory);
            }

            dir->createFile(path.filename());
        } catch (std::system_error &e) {
            return -e.code().value();
        }

        return 0;
    };

    fuseOps->rename = [](const char *from, const char *to) -> int {
        try {
            auto entity = instance().getPath(from);
            std::filesystem::path target{to};
            auto targetEntity = instance().getPath(target.parent_path());
            auto targetDir = dynamic_cast<Directory*>(targetEntity.get());

            if (!targetDir) {
                throwError(std::errc::not_a_directory);
            }

            entity->move(*targetDir, target.filename());
        } catch (std::system_error &e) {
            return -e.code().value();
        }
        return 0;
    };


    fuseOps->read = [](const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) -> int {
        // removes unused fi warning
        (void) fi;

        try {
            auto entity = instance().getPath(path);
            auto* file = dynamic_cast<File*>(entity.get());

            if (!file) {
                throwError(std::errc::is_a_directory);
            }

            auto len = file->getSize();

            if (offset < len) {
                if (len < offset + size) {
                    size = len - offset;
                }
                std::memcpy(buf, file->getContents().data() + offset, size);
            } else {
                size = 0;
            }
        } catch (std::system_error &e) {
            return -e.code().value();
        }

        return size;
    };

    fuseOps->write = [](const char *path, const char *buf, size_t size, off_t offset,
                        struct fuse_file_info *fi) -> int {
        (void) fi;

        try {
            auto entity = instance().getPath(path);
            auto* file = dynamic_cast<File*>(entity.get());

            if (!file) {
                throwError(std::errc::is_a_directory);
            }

            file->write(buf, size, offset);
        } catch (std::system_error &e) {
            return -e.code().value();
        }

        return size;
    };

    fuseOps->unlink = [](const char *pathStr) -> int {
        try {
            auto path = std::filesystem::path(pathStr);
            auto entity = instance().getPath(path.parent_path());
            auto* dir = dynamic_cast<Directory*>(entity.get());

            if (!dir) {
                throwError(std::errc::no_such_file_or_directory);
            }

            dir->deleteFile(path.filename());
        } catch (std::system_error &e) {
            return -e.code().value();
        }

        return 0;
    };

    fuseOps->rmdir = [](const char *pathStr) -> int {
        try {
            auto path = std::filesystem::path(pathStr);
            auto entity = instance().getPath(path.parent_path());
            auto* dir = dynamic_cast<Directory*>(entity.get());

            if (!dir) {
                throwError(std::errc::no_such_file_or_directory);
            }

            dir->deleteDirectory(path.filename());
        } catch (std::system_error &e) {
            return -e.code().value();
        }

        return 0;
    };

    fuseOps->mkdir = [](const char *pathStr, mode_t mode) -> int {
        (void)mode;

        try {
            auto path = std::filesystem::path(pathStr);
            auto parent = path.parent_path();
            auto name = path.filename();
            auto entity = instance().getPath(parent);
            auto* dir = dynamic_cast<Directory*>(entity.get());

            if (!dir) {
                throwError(std::errc::not_a_directory);
            }

            dir->createDirectory(name);

            return 0;
        } catch (std::system_error &e) {
            return -e.code().value();
        }
    };
}

BabylonFS &BabylonFS::instance() noexcept {
    static BabylonFS singleton;
    return singleton;
}

std::string BabylonFS::getSeed() noexcept {
    return instance().seed;
}
