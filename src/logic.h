#pragma once

#include <utility>
#include "babylonfs.h"

struct RoomData;
struct Bookcase;
struct Shelf;
struct Book;

struct Book : public File {
    std::string name;
    std::string contents;

    explicit Book(const std::string &name, RoomData *myRoom, std::string shelf_name);
    std::string_view getContents() override;
    int getSize() override;
    void move(Entity &to, const std::string& newName) override;

    RoomData *myRoom;
    std::string shelfName;
};

struct Shelf : public Directory {
    explicit Shelf(std::string name, RoomData* myRoom);
    void move(Entity &to, const std::string& newName) override;
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;

    RoomData* myRoom;
};

struct Bookcase : Directory {
    Bookcase(std::string name, RoomData* myRoom);
    void move(Entity &to, const std::string& newName) override;
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;

    RoomData* myRoom;
};

struct Desk : public Directory {
    explicit Desk(RoomData* myRoom);
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;
    void createFile(std::string name) override;
    void deleteFile(const std::string &name) override;
    void createDirectory(const std::string &name) override;

    void deleteDirectory(const std::string &name) override;

    RoomData* myRoom;
};

struct Notes : public Directory {
    Notes(std::string name, RoomData* myRoom);
    std::vector<std::string> getContents() override;
    ptr get(const std::string &name) override;
    void createFile(std::string name) override;
    void deleteFile(const std::string &name) override;

    RoomData* myRoom;
};

struct Note : public File {
public:
    Note(const std::string &name, int id, RoomData* myRoom, bool isBasket, std::string  basketName);
    std::string_view getContents() override;
    void write(const char *buf, size_t size, off_t offset) override; // TODO: implement me
    void move(Entity &to, const std::string& newName) override;

    int id;
    bool isBasket;
    RoomData* myRoom;
    std::string basketName;
};

struct RoomData {
    RoomData(int n, int cycle);

    int cycle;
    int leftN;
    int rightN;
    std::unordered_map<std::string, std::vector<NoteContent>> myBaskets;
    std::vector<NoteContent> myNotes;
    std::unordered_map<std::string, std::vector<std::string>> takenBooks;
    std::unordered_map<std::string, std::vector<std::string>> shelfToBook;
};

struct Room : public Directory {
    explicit Room(RoomData*);

    std::vector<std::string> getContents() override;
    Entity::ptr get(const std::string &name) override;

private:
    RoomData *data;
};

struct RoomStorage {
    explicit RoomStorage(int cycle);
    RoomData* getRoom(int n);

private:
    int cycle;
    std::unordered_map<int, std::unique_ptr<RoomData>> rooms;
};
