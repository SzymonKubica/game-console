#ifdef EMULATOR
#include "../interface/persistent_storage.hpp"

template <typename T> T &PersistentStorage::get(int offset, T &t) {}
template <typename T> const T &PersistentStorage::put(int offset, const T &t) {}
#endif
