#pragma once

#include <utility>
#include "babylonfs.h"

struct Room;
struct Bookcase;
struct Shelf;
struct Book;

struct Book : public File {
    std::string name;
    std::string contents;

    explicit Book(const std::string &name, Room *myRoom, std::string shelf_name);
    std::string_view getContents() override;
    void move(Entity &to) override;

    Room *myRoom;
    std::string shelf_name;
};

struct Shelf : public Directory {
    explicit Shelf(std::string name, Room* myRoom);
    void rename(const std::string &to) override;
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;

    Room* myRoom;
};

struct Bookcase : Directory {
    Bookcase(std::string name, Room* myRoom);
    void rename(const std::string &to) override;
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;

    Room* myRoom;
};

struct Desk : public Directory {
    explicit Desk(Room* myRoom);
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;
    void createFile(std::string name) override;
    void deleteFile(const std::string &name) override;
    void createDirectory(const std::string &name) override;

    void deleteDirectory(const std::string &name) override;

    Room* myRoom;
};

struct Notes : public Directory {
    Notes(std::string name, Room* myRoom);
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;
    void createFile(std::string name) override;
    void deleteFile(const std::string &name) override; //todo [masha F]

    Room* myRoom;
};

struct Note : public File {
public:
    Note(const std::string &name, int id, Room* myRoom, bool isBasket, std::string  basketName);
    std::string_view getContents() override;
    void write(const char *buf, size_t size, off_t offset) override; // TODO: implement me
    void rename(const std::string &to) override;
    void move(Entity &to) override; //todo [masha F]

    int id;
    bool isBasket;
    Room* myRoom;
    std::string basketName;
};

struct Room : public Directory {
    Room(int n, int cycle);

    //todo [masha F] запретить создание штук с зарезервированными именами
    std::vector<std::string> getContents() override;
    Entity::ptr get(const std::string &name) override;

    int cycle;
    int left_n;
    int right_n;
    std::unordered_map<std::string, std::vector<note_content>> myBaskets;
    std::vector<note_content> myNotes;
    std::unordered_map<std::string, std::vector<std::string>> taken_books;
    std::unordered_map<std::string, std::vector<std::string>> shelf_to_book;
};
