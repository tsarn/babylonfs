#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <fuse.h>

using note_content = std::pair<std::string, std::string>;

struct Entity {
    using ptr = std::unique_ptr<Entity>;

    virtual void stat(struct stat *) = 0;

    virtual void rename(const std::string &to) {
        //TODO error
    }

    //assume to is new parent path
    virtual void move(const std::string &to) {
        //TODO error
    }

    virtual ~Entity() = default;

protected:
    std::string name;
};

struct Directory : public Entity {
    void stat(struct stat *) override;

    virtual std::vector<std::string> getContents() = 0;

    virtual Entity::ptr get(const std::string &name) = 0;

    virtual void mkdir(const std::string &name) {
        //TODO error
    }

    virtual void create(const std::string &name) {
        //TODO error
    }
};

struct File : public Entity {
    void stat(struct stat *) override;

    virtual std::string_view getContents() = 0;

    virtual void write(const char *buf, size_t size, off_t offset) {
        //TODO error
    }
};


class BabylonFS {
public:
    static const struct fuse_operations *run(const char *seed) noexcept;

private:
    BabylonFS();

    BabylonFS(int cycle) : cycle(cycle) {};

    static BabylonFS &instance() noexcept;

    Entity::ptr getRoot();

    Entity::ptr getPath(const char *pathStr);

private:
    std::unique_ptr<struct fuse_operations> fuseOps{};
    std::string seed;
    int cycle = -1;
};
