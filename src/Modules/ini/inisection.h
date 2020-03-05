#pragma once

#include "Interfaces/filesystem/path.h"

namespace Engine {
namespace Ini {

    struct MODULES_EXPORT IniSection {
        IniSection(std::istream *stream = nullptr);

        std::string &operator[](const std::string &key);

        void save(std::ostream &stream) const;
        void load(std::istream &stream);

        std::map<std::string, std::string>::iterator begin();
        std::map<std::string, std::string>::iterator end();

    private:
        std::map<std::string, std::string> mValues;
    };

}
}