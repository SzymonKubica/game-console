#pragma once
class PersistentStorage
{
      public:
        template <typename T> T &get(int offset, T &t);
        template <typename T> const T &put(int offset, const T &t);
};
