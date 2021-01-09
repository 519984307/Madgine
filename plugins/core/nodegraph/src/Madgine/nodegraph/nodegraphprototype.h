#pragma once

#include "Interfaces/filesystem/path.h"

#include "graphexecutable.h"

#include "Modules/serialize/container/serializablecontainer.h"

namespace Engine {
namespace NodeGraph {

    struct MADGINE_NODEGRAPH_EXPORT NodeGraphPrototype : ScopeBase, GraphExecutable {

        SERIALIZABLEUNIT(NodeGraphPrototype);

        NodeGraphPrototype();
        NodeGraphPrototype(const NodeGraphPrototype &) = delete;
        NodeGraphPrototype(NodeGraphPrototype &&);
        ~NodeGraphPrototype();

        NodeGraphPrototype &operator=(const NodeGraphPrototype &) = delete;

        void loadFromFile(const Filesystem::Path &path);
        void saveToFile();

        NodePrototypeBase *addNode(std::unique_ptr<NodePrototypeBase> node);
        NodePrototypeBase *addNode(const std::string_view &name);
        void removeNode(NodePrototypeBase *node);

        const std::vector<std::unique_ptr<NodePrototypeBase>> &nodes() const;

        void connectFlow(NodePrototypeBase *source, uint32_t sourceIndex, GraphExecutable *target, uint32_t targetIndex);
        void connectData(GraphExecutable *source, uint32_t sourceIndex, NodePrototypeBase *target, uint32_t targetIndex);

        
        virtual void execute(NodeInstance *instance, uint32_t flowIndex, ArgumentList *out = nullptr) const override;

        NodeGraphInstance createInstance();

        std::vector<FlowOutPinPrototype> mFlowInPins;
        std::vector<DataOutPinPrototype> mDataInPins;
        size_t mFlowOutPinCount = 0;
        std::vector<DataInPinPrototype> mDataOutPins;

    protected:
        std::unique_ptr<NodePrototypeBase> createNode(const std::string_view &name);
        std::tuple<std::unique_ptr<NodePrototypeBase>> createNodeTuple(const std::string &name);
        std::tuple<std::pair<const char *, std::string_view>> storeNodeCreationData(const std::unique_ptr<NodePrototypeBase> &node) const;

    private:
        Filesystem::Path mPath;

        SERIALIZABLE_CONTAINER(mNodes, std::vector<std::unique_ptr<NodePrototypeBase>>, NoOpFunctor);
    };

}
}
