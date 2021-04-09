#include "babylonfs.h"

struct TestFile : File {
    std::string contents;

    explicit TestFile(const std::string& contents) : contents(contents) {}

    std::string_view getContents() const override {
        return contents;
    };
};

struct TestDirectory : Directory {
    int depth;

    explicit TestDirectory(int depth) : depth(depth) {}

    std::vector<std::string> getContents() const override {
        if (depth == 0) {
            return { "the_end" };
        }
        return {"a", "b"};
    }

    Entity::ptr get(const std::string& name) const override {
        if (name == "a") {
            return std::make_unique<TestDirectory>(depth - 1);
        } else if (name == "b") {
            return std::make_unique<TestFile>("Hi! My name is b");
        } else if (name == "the_end") {
            return std::make_unique<TestFile>("This is the end for you, my friend");
        } else {
            return nullptr;
        }
    }
};

Entity::ptr BabylonFS::getRoot() const {
    return std::make_unique<TestDirectory>(5);
}
