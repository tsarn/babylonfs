#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <fuse.h>

using NoteContent = std::pair<std::string, std::string>;

[[noreturn]] void throwError(std::errc code);

struct Entity {
    using ptr = std::unique_ptr<Entity>;

    virtual void stat(struct stat *) = 0;

    virtual void move(Entity &to, const std::string& newName);

    virtual ~Entity() = default;

    std::string name;
};

struct Directory : public Entity {
    void stat(struct stat *) override;

    virtual std::vector<std::string> getContents() = 0;

    virtual Entity::ptr get(const std::string &name) = 0;

    virtual void createFile(const std::string& name);

    virtual void deleteFile(const std::string &name);

    virtual void createDirectory(const std::string &name);

    virtual void deleteDirectory(const std::string &name);
};

struct File : public Entity {
    void stat(struct stat *) override;

    virtual std::string_view getContents() = 0;

    virtual int getSize();

    virtual bool isWriteable();

    virtual void write(const char *buf, size_t size, off_t offset);
};


class BabylonFS {
public:
    static const struct fuse_operations *run(const char *seed) noexcept;

private:
    BabylonFS();

    BabylonFS(int cycle) : cycle(cycle) {};

    static BabylonFS &instance() noexcept;

    Entity::ptr getRoot();

    Entity::ptr getPath(const std::string& pathStr);

private:
    std::unique_ptr<struct fuse_operations> fuseOps{};
    std::string seed;
    int cycle = -1;
};
