#include "babylonfs.h"
#include "logic.h"

Entity::ptr BabylonFS::getRoot() {
    return std::make_unique<Room>(0, false);
}

Book::Book(const std::string &name, Room *myRoom) : myRoom(myRoom) {
    this->name = name;
    //todo random contents by name as a seed
}

std::string_view Book::getContents() {
    return contents;
}

void Book::move(const std::string &to) {
    //todo [masha F]
}

Shelf::Shelf(std::string name, Room *myRoom) : myRoom(myRoom) {
    this->name = name;
}

void Shelf::rename(const std::string &to) {
    // "does nothing"
}

Bookcase::Bookcase(std::string name, Room *myRoom) : myRoom(myRoom) {
    this->name = name;
}

void Bookcase::rename(const std::string &to) {
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
    return res;
}

Desk::Desk(Room *myRoom) : myRoom(myRoom) {}

void Desk::mkdir(const std::string &name) {
    myRoom->myBaskets[name] = {};
}

Notes::Notes(std::string name, Room *myRoom) : myRoom(myRoom) {
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

Note::Note(const std::string &name, int id, Room *myRoom, bool isBasket, std::string basketName) :
    id(id), myRoom(myRoom), isBasket(isBasket), basketName(std::move(basketName)) {
    this->name = name;
}

Room::Room(int n, int cycle) : cycle(cycle) {
    if (cycle == 0) {
        left_n = n - 1;
        right_n = n + 1;
    } else {
        if (n == 0) {
            left_n = cycle - 1;
            right_n = 1;
        } else if (n == cycle - 1) {
            left_n = n - 1;
            right_n = 0;
        }
    }
    for (auto kek : {"b0", "b1", "b2","b3"}) {
        for (auto kek2 : {"0", "1", "2", "3", "4"}) {
            auto shelf_name = std::string(kek) + kek2;
            auto seed = std::to_string(n) + kek + kek2;
            std::vector<std::string> names(32); //todo generate names from seed
            shelf_to_book[shelf_name] = names;
        }
    }
}

std::vector<std::string> Room::getContents() {
    return {
            std::to_string(left_n),
            std::to_string(right_n),
            "0", "1", "2", "3", "desk"
    };
}

Entity::ptr Room::get(const std::string &name) {
    if (name == "b0" || name == "b1" || name == "b2" || name == "b3") {
        return std::make_unique<Bookcase>(name, this);
    } else if (name == std::to_string(left_n)) {
        return std::make_unique<Room>(left_n, cycle);
    } else if (name == std::to_string(right_n)) {
        return std::make_unique<Room>(right_n, cycle);
    } else if (name == "desk") {
        return std::make_unique<Desk>(this);
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

std::vector<std::string> Shelf::getContents() {
    return myRoom->shelf_to_book.at(this->name);
}

Entity::ptr Shelf::get(const std::string &name) {
    auto book_names = myRoom->shelf_to_book.at(this->name);
    for (const auto &kek: book_names) {
        if (kek == name) {
            return std::make_unique<Book>(name, myRoom);
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
}

std::string_view Note::getContents() {
    if (isBasket) {
        std::vector<note_content> notes = myRoom->myNotes;
        return notes[id].second;
    } else {
        std::vector<note_content> notes = myRoom->myBaskets.at(basketName);
        return notes[id].second;
    }
}

void Note::rename(const std::string &to) {
    if (isBasket) {
        std::vector<note_content> notes = myRoom->myNotes;
        notes[id].first = to;
    } else {
        std::vector<note_content> notes = myRoom->myBaskets.at(basketName);
        notes[id].first = to;
    }
}
