#include "babylonfs.h"
#include "logic.h"
#include "util.h"

#include <utility>

static const int bookSize = 4096 * 256;

Entity::ptr BabylonFS::getRoot() {
    static RoomStorage roomStorage{cycle};
    return std::make_unique<Room>(roomStorage.getRoom(0));
}

Book::Book(const std::string &name, RoomData *myRoom, std::string shelf_name) : myRoom(myRoom), shelf_name(std::move(shelf_name)) {
    this->name = name;
}

int Book::getSize() {
    return bookSize;
}

std::string_view Book::getContents() {
    if (contents.empty()) {
        contents = generateStringFromSeed(name, bookSize);
    }
    return contents;
}

void Book::move(Entity &to) {
    if (auto shelf = dynamic_cast<Shelf *>(&to)) {
        if (myRoom != shelf->myRoom) {
            throwError(std::errc::invalid_argument);
        }
        if (shelf->name == shelf_name) {
            auto taken_books = myRoom->takenBooks[shelf_name];
            auto it = std::find(taken_books.begin(), taken_books.end(), name);
            if (std::find(taken_books.begin(), taken_books.end(), name) == taken_books.end()) {
                throwError(std::errc::invalid_argument);
            } else {
                myRoom->shelfToBook[shelf_name].push_back(name);
            }
            taken_books.erase(it);
        } else {
            throwError(std::errc::permission_denied);
        }
    } else if (auto desk = dynamic_cast<Desk *>(&to)) {
        if (myRoom != desk->myRoom) {
            throwError(std::errc::invalid_argument);
        }
        auto shelf_books = myRoom->shelfToBook[shelf_name];
        auto it = std::find(shelf_books.begin(), shelf_books.end(), name);
        if (it == shelf_books.end()) {
            throwError(std::errc::invalid_argument);
        } else {
            myRoom->takenBooks[shelf_name].push_back(name);
        }
        shelf_books.erase(it);
    }
}

Shelf::Shelf(std::string name, RoomData *myRoom) : myRoom(myRoom) {
    this->name = name;
}

void Shelf::rename(const std::string &to) {
    (void)to;
    // "does nothing"
}

Bookcase::Bookcase(std::string name, RoomData *myRoom) : myRoom(myRoom) {
    this->name = name;
}

void Bookcase::rename(const std::string &to) {
    (void)to;
    // "does nothing"
}

std::vector<std::string> Bookcase::getContents() {
    return {"0", "1", "2", "3", "4"};
}

Entity::ptr Bookcase::get(const std::string &name) {
    if (name == "0" || name == "1" || name == "2" || name == "3" || name == "4") {
        return std::make_unique<Shelf>(this->name + name, myRoom);
    }
    return nullptr;
}

std::vector<std::string> Desk::getContents() {
    std::vector<std::string> res;

    for(const auto &kek : myRoom->myNotes) {
        res.push_back(kek.first);
    }

    for(const auto &kek: myRoom->myBaskets) {
        res.push_back(kek.first);
    }
    for (const auto &kek: myRoom->takenBooks) {
        for (const auto &kek2: kek.second) {
            res.push_back(kek2);
        }
    }
    return res;
}

Desk::Desk(RoomData *myRoom) : myRoom(myRoom) {}

void Desk::createDirectory(const std::string &name) {
    auto contents = getContents();
    auto it = std::find(contents.begin(), contents.end(), name);
    if (it != contents.end()) {
        throwError(std::errc::invalid_argument);
    }
    myRoom->myBaskets[name] = {};
}

Notes::Notes(std::string name, RoomData *myRoom) : myRoom(myRoom) {
    this->name = name;
}

std::vector<std::string> Notes::getContents() {
    std::vector<std::string> res;
    auto myNotes = myRoom->myBaskets.at(this->name);
    for (const auto &kek: myNotes) {
        res.push_back(kek.first);
    }
    return res;
}

Note::Note(const std::string &name, int id, RoomData *myRoom, bool isBasket, std::string basketName) :
    id(id), isBasket(isBasket), myRoom(myRoom), basketName(std::move(basketName)) {
    this->name = name;
}

RoomData::RoomData(int n, int cycle) : cycle(cycle) {
    if (cycle == -1) {
        leftN = n - 1;
        rightN = n + 1;
    } else {
        if (n == 0) {
            leftN = cycle - 1;
            rightN = 1;
        } else if (n == cycle - 1) {
            leftN = n - 1;
            rightN = 0;
        }
    }
    for (auto kek : {"b0", "b1", "b2","b3"}) {
        for (auto kek2 : {"0", "1", "2", "3", "4"}) {
            auto shelfName = std::string(kek) + kek2;
            auto seed = std::to_string(n) + kek + kek2;
            std::vector<std::string> names(32);
            for (int i = 0; i < names.size(); ++i) {
                names[i] = generateStringFromSeed(shelfName + "/book/" + std::to_string(i), 16);
            }
            shelfToBook[shelfName] = names;
        }
    }
}

Room::Room(RoomData* data) : data(data) {}

std::vector<std::string> Room::getContents() {
    return {
            "k" + std::to_string(data->leftN),
            "k" + std::to_string(data->rightN),
            "b0", "b1", "b2", "b3", "desk"
    };
}

Entity::ptr Room::get(const std::string &name) {
    if (name == "b0" || name == "b1" || name == "b2" || name == "b3") {
        return std::make_unique<Bookcase>(name, data);
    } else if (name == "k" + std::to_string(data->leftN)) {
        return std::make_unique<Room>(data);
    } else if (name == "k" + std::to_string(data->rightN)) {
        return std::make_unique<Room>(data);
    } else if (name == "desk") {
        return std::make_unique<Desk>(data);
    }
    return nullptr;
}

Entity::ptr Notes::get(const std::string &name) {
    int id = -1;
    auto myNotes = myRoom->myBaskets[this->name];
    for (size_t i = 0; i < myNotes.size(); ++i) {
        if (myNotes[i].first == name) {
            id = i;
            break;
        }
    }
    if (id == -1) return nullptr;
    return std::make_unique<Note>(name, id, myRoom, true, this->name);
}

void Notes::createFile(std::string name) {
    auto contents = getContents();
    auto it = std::find(contents.begin(), contents.end(), name);
    if (it != contents.end()) {
        throwError(std::errc::invalid_argument);
    }
    NoteContent me;
    me.first = name;
    me.second = {};
    myRoom->myBaskets[this->name].push_back(me);
}

void Notes::deleteFile(const std::string &name) {
    auto notes = myRoom->myBaskets[this->name];
    int id = -1;
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i].first == name) {
            id = i;
        }
    }
    if (id == -1) {
        throwError(std::errc::invalid_argument);
    } else {
        notes.erase(notes.begin() + id);
    }
}

std::vector<std::string> Shelf::getContents() {
    return myRoom->shelfToBook.at(this->name);
}

Entity::ptr Shelf::get(const std::string &name) {
    auto book_names = myRoom->shelfToBook.at(this->name);
    for (const auto &kek: book_names) {
        if (kek == name) {
            return std::make_unique<Book>(name, myRoom, this->name);
        }
    }
    return nullptr;
}

Entity::ptr Desk::get(const std::string &name) {
    if (myRoom->myBaskets.contains(name)) {
        return std::make_unique<Notes>(name, myRoom);
    }

    for (size_t i = 0; i < myRoom->myNotes.size(); ++i) {
        if (myRoom->myNotes[i].first == name) {
            return std::make_unique<Note>(name, i, myRoom, false, "");
        }
    }

    for (const auto &kek: myRoom->takenBooks) {
        for (const auto &kek2: kek.second) {
            if (kek2 == name) {
                return std::make_unique<Book>(name, myRoom, kek.first);
            }
        }
    }
    
    return nullptr;
}

void Desk::createFile(std::string name) {
    auto contents = getContents();
    auto it = std::find(contents.begin(), contents.end(), name);
    if (it != contents.end()) {
        throwError(std::errc::invalid_argument);
    }
    NoteContent me;
    me.first = name;
    me.second = {};
    myRoom->myNotes.push_back(me);
}

void Desk::deleteFile(const std::string &name) {
    auto notes = myRoom->myNotes;
    int id = -1;
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i].first == name) {
            id = i;
        }
    }
    if (id == -1) {
        throwError(std::errc::invalid_argument);
    } else {
        notes.erase(notes.begin() + id);
    }
}

void Desk::deleteDirectory(const std::string &name) {
    auto notes = myRoom->myBaskets;
    if (notes.contains(name)) {
        notes.erase(name);
    } else {
        throwError(std::errc::invalid_argument);
    }
}

std::string_view Note::getContents() {
    if (isBasket) {
        std::vector<NoteContent> notes = myRoom->myNotes;
        return notes[id].second;
    } else {
        std::vector<NoteContent> notes = myRoom->myBaskets.at(basketName);
        return notes[id].second;
    }
}

void Note::rename(const std::string &to) {
    if (isBasket) {
        std::vector<NoteContent> notes = myRoom->myNotes;
        notes[id].first = to;
    } else {
        std::vector<NoteContent> notes = myRoom->myBaskets.at(basketName);
        notes[id].first = to;
    }
}

void Note::move(Entity &to) {
    NoteContent me;
    if (isBasket) {
        std::vector<NoteContent> notes = myRoom->myNotes;
        me = notes[id];
        notes.erase(notes.begin() + id);
    } else {
        std::vector<NoteContent> notes = myRoom->myBaskets.at(basketName);
        me = notes[id];
        notes.erase(notes.begin() + id);
    }
    if (dynamic_cast<Notes *>(&to) != nullptr) {
        auto kek = dynamic_cast<Notes *>(&to);
        if (myRoom != kek->myRoom) {
            throwError(std::errc::invalid_argument);
        }
        this->myRoom->myBaskets[kek->name].push_back(me);
    } else if (dynamic_cast<Desk *>(&to) != nullptr) {
        auto kek = dynamic_cast<Desk *>(&to);
        if (myRoom != kek->myRoom) {
            throwError(std::errc::invalid_argument);
        }
        this->myRoom->myNotes.push_back(me);
    }
}

void Note::write(const char *buf, size_t size, off_t offset) {
    NoteContent me;

    if (isBasket) {
        std::vector<NoteContent> notes = myRoom->myNotes;
        me = notes[id];
    } else {
        std::vector<NoteContent> notes = myRoom->myBaskets.at(basketName);
        me = notes[id];
    }

    if (offset + size > me.second.size()) {
        me.second.resize(offset + size);
    }

    for (size_t i = 0; i < size; ++i) {
        me.second[offset + i] = buf[i];
    }
}

RoomStorage::RoomStorage(int cycle) : cycle(cycle) {}

RoomData* RoomStorage::getRoom(int n) {
    if (rooms[n]) return rooms[n].get();
    return (rooms[n] = std::make_unique<RoomData>(n, cycle)).get();
}
