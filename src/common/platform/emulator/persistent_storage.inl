#ifdef EMULATOR
#include <fstream>
#include <iostream>

template <typename T> T &PersistentStorage::get(int offset, T &t)
{
        std::ifstream ifs("persistent_storage.bin", std::ios::binary);

        if (!ifs) {
                std::cerr << "Error opening file for writing." << std::endl;
                return t; // Return the original object if file opening fails
        }

        ifs.seekg(offset, std::ios::beg);
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

        ofs.seekp(offset, std::ios::beg);
        ofs.write(reinterpret_cast<const char *>(&t), sizeof(T));
        return t;
}
#endif
