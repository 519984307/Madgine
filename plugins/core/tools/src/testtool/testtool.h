#pragma once

#include "../toolbase.h"
#include "../toolscollector.h"

namespace Engine {
namespace Tools {

    struct TestTool : Tool<TestTool> {

        TestTool(ImRoot &root);

        virtual void render() override;
        virtual void update() override;

        std::string_view key() const override;

        void logTest();
        void logValue(const ValueType &v);
        int dummy();

    private:
        int mSphereDetail = 1;
    };

}
}

RegisterType(Engine::Tools::TestTool);