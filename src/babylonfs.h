#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <vector>

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

struct Bookcase : Directory {
    void rename(const std::string &to) override {
        //TODO
    }

    std::vector<std::string> getContents() const override {
        //TODO
        return std::vector<std::string>();
    }

    ptr get(const std::string &name) const override {
        //TODO
        return Entity::ptr();
    }

private:
    std::string name;
};

struct Shelf : Directory {
    void rename(const std::string &to) override {
        //TODO
    }

    std::vector<std::string> getContents() const override {
        //TODO
        return std::vector<std::string>();
    }

    ptr get(const std::string &name) const override {
        //TODO
        return Entity::ptr();
    }
};

struct Book : public File {
    std::string contents;

    explicit Book(const std::string &contents) : contents(contents) {}

    std::string_view getContents() const override {
        return contents;
    };
    Shelf* myShelf;
};

//TODO all + move
struct Desk : public Directory {
    std::vector<std::string> getContents() const override {
        return std::vector<std::string>();
    }

    ptr get(const std::string &name) const override {
        return Entity::ptr();
    }

private:
    //todo pointers?
    std::vector<Book*> myBooks;
    std::vector<Entity*> myContents;
};

//TODO all + move
struct Notes : public Directory {
    std::vector<std::string> getContents() const override {
        return std::vector<std::string>();
    }

    ptr get(const std::string &name) const override {
        return Entity::ptr();
    }

};

struct Note : File {
public:
    explicit Note(const std::string &contents) : contents(contents) {}

    std::string_view getContents() const override {
        return contents;
    };

    void write(const char *buf, size_t size, off_t offset)  override {
        //TODO I don't know c++ and it seems like I will make an error anyway so TODO
    }

    void rename(const std::string &to) override {
        File::rename(to);
    };

private:
    std::string contents;
};

struct Room : Directory {
    int depth;
    std::pair<Room*, Room*> neighbours;

    explicit Room() : depth(-1) {}

    explicit Room(int depth) : depth(depth) {}

    std::vector <std::string> getContents() const override {
        if (depth == 0) {
            return {"the_end"};
        }
        return {"a", "b"};
    }

    Entity::ptr get(const std::string &name) const override {
        if (name == "a") {
            return std::make_unique<Room>(depth - 1);
        } else if (name == "b") {
            return std::make_unique<Note>("Hi! My name is b");
        } else if (name == "the_end") {
            return std::make_unique<Note>("This is the end for you, my friend");
        } else {
            return nullptr;
        }
    }

    void mkdir(const std::string &name) override {
    }

    void rename(const std::string &to) override {
        Directory::rename(to);
    }
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
