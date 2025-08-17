#ifdef EMULATOR
#include <fstream>
#include <iostream>

// #include <nlohmann/json.hpp> // This will be used later for json
// serialization that can be used for debugging.

template <typename T> T &PersistentStorage::get(int offset, T &t)
{
        std::ifstream ifs("persistent_storage.bin", std::ios::binary);

        if (!ifs) {
                std::cerr << "Error opening file for writing." << std::endl;
                return t; // Return the original object if file opening fails
        }

        ifs.seekg(offset);
        ifs.read(reinterpret_cast<char *>(&t), sizeof(T));

        return t;
}

template <typename T> const T &PersistentStorage::put(int offset, const T &t)
{
        std::ofstream ofs("persistent_storage.bin", std::ios::binary);

        if (!ofs) {
                std::cerr << "Error opening file for writing." << std::endl;
                return t; // Return the original object if file opening fails
        }

        ofs.write(reinterpret_cast<const char *>(&t), sizeof(T));
        return t;
}
#endif
