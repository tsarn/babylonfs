#include "babylonfs.h"

Entity::ptr BabylonFS::getRoot() const {
    return std::make_unique<Room>(5);
}
