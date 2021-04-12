#include "babylonfs.h"

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
    std::string name
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

struct Book : File {
    std::string contents;

    explicit Book(const std::string &contents) : contents(contents) {}

    std::string_view getContents() const override {
        return contents;
    };
    Shelf* myShelf;
};

//TODO all + move
struct Desk : Directory {
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
struct Notes : Directory {
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
};

Entity::ptr BabylonFS::getRoot() const {
    return std::make_unique<Room>(5);
}
