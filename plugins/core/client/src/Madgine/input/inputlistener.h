#pragma once

namespace Engine {
namespace Input {

    class InputListener {
    public:
        virtual bool injectKeyPress(const KeyEventArgs &arg) { return false; };
        virtual bool injectKeyRelease(const KeyEventArgs &arg) { return false; }
        virtual bool injectPointerPress(const PointerEventArgs &arg) { return false; }
        virtual bool injectPointerRelease(const PointerEventArgs &arg) { return false; }
        virtual bool injectPointerMove(const PointerEventArgs &arg) { return false; }
    };

}
}