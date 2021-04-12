#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include "logic.cpp"

//TODO defaults stuff for rename / write
struct Entity {
    using ptr = std::unique_ptr<Entity>;
    virtual void stat(struct stat*) const = 0;
    virtual void rename(const std::string& to) = 0;
    virtual ~Entity() = default;
};

struct Directory : public Entity {
    void stat(struct stat*) const override;
    virtual std::vector<std::string> getContents() const = 0;
    virtual Entity::ptr get(const std::string& name) const = 0;
    void rename(const std::string& to) override = 0;
    virtual void mkdir(const std::string& name) = 0;
};

struct File : public Entity {
    void stat(struct stat*) const override;
    virtual std::string_view getContents() const = 0;
    virtual void write(const char *buf, size_t size, off_t offset) = 0;
    void rename(const std::string& to) override = 0;
};

class BabylonFS {
public:
    static const struct fuse_operations* run(const char* seed) noexcept;

private:
    BabylonFS();
    static BabylonFS& instance() noexcept;
    Entity::ptr getRoot() const;
    Entity::ptr getPath(const char* pathStr) const;

private: 
    std::unique_ptr<struct fuse_operations> fuseOps{};
    std::string seed;
    std::vector<std::pair<int, int>> topology;
    std::vector<Room> rooms;
};
