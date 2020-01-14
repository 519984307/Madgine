#pragma once

#include "../toolscollector.h"

#include "Modules/debug/history.h"

namespace Engine {
namespace Tools {

    struct Metrics : Tool<Metrics> {
        Metrics(ImRoot &root);

        virtual void render() override;
        virtual void update() override;

        const char *key() const override;

    private:
        Debug::History<float, 120> mFramesPerSecond;

        Debug::History<float, 100> mFramesTrend;
        float mTimeBank;
    };

}
}