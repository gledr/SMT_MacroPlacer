#ifndef PIN_DEFINITION_HPP
#define PIN_DEFINITION_HPP

#include <string>

namespace Placer {

struct PinDefinition {
    std::string parent;
    std::string name;
    std::string direction;
};

}

#endif /* PIN_DEFINITION_HPP */
