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
        throw domain_error("rename is forbidden for this entity");
    }

    //assume to is new parent path
    virtual void move(Entity &to) {
        throw domain_error("move is forbidden for this entity");
    }

    virtual ~Entity() = default;

    std::string name;
};

struct Directory : public Entity {
    void stat(struct stat *) override;

    virtual std::vector<std::string> getContents() = 0;

    virtual Entity::ptr get(const std::string &name) = 0;

    virtual void mkdir(const std::string &name) {
        throw domain_error("mkdir is forbidden for this directory");
    }

    virtual void create(const std::string &name) {
        throw domain_error("create is forbidden for this directory");
    }
};

struct File : public Entity {
    void stat(struct stat *) override;

    virtual std::string_view getContents() = 0;

    virtual void write(const char *buf, size_t size, off_t offset) {
        throw domain_error("write is forbidden for this file");
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
