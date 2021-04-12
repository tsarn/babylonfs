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

    explicit Book(const std::string &name, Room *myRoom);
    std::string_view getContents() override;
    void move(const std::string &to) override;

private:
    Room *myRoom;
};

struct Shelf : public Directory {
    explicit Shelf(std::string name, Room* myRoom);
    void rename(const std::string &to) override;
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;

private:
    Room* myRoom;
};

struct Bookcase : Directory {
    Bookcase(std::string name, Room* myRoom);
    void rename(const std::string &to) override;
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;
private:
    Room* myRoom;
};

struct Desk : public Directory {
    explicit Desk(Room* myRoom);
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;
    void mkdir(const std::string &name) override;
    void create(const std::string &name) override; //todo [masha F]

private:
    Room* myRoom;
};

struct Notes : public Directory {
    Notes(std::string name, Room* myRoom);
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;
    void create(const std::string &name) override; //todo [masha F]

private:
    Room* myRoom;
};

struct Note : public File {
public:
    Note(const std::string &name, int id, Room* myRoom, bool isBasket, std::string  basketName);
    std::string_view getContents() override;
    void write(const char *buf, size_t size, off_t offset) override; // TODO: implement me
    void rename(const std::string &to) override;
    void move(const std::string &to) override; //todo [masha F]

private:
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
    std::unordered_map<std::string, std::vector<std::string>> shelf_to_book;
};
