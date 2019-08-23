#pragma once

#include "../input/inputlistener.h"

namespace Engine {
namespace GUI {

    struct WindowOverlay : Input::InputListener {
        virtual void render() {}
        virtual void calculateAvailableScreenSpace(Window::Window *w, Vector3 &pos, Vector3 &size) {}
    };

}
}