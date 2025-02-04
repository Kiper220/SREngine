//
// Created by Monika on 01.08.2022.
//

#ifndef SRENGINE_ANCHOR_H
#define SRENGINE_ANCHOR_H

#include <Utils/ECS/Component.h>

namespace SR_GRAPH_NS::UI {
    class Canvas;

    class Anchor : public SR_UTILS_NS::Component {
        SR_ENTITY_SET_VERSION(1000);
        SR_INITIALIZE_COMPONENT(Anchor);
    public:
        Anchor();
        ~Anchor() override = default;

    public:
        static SR_UTILS_NS::Component* LoadComponent(SR_HTYPES_NS::Marshal& marshal, const SR_HTYPES_NS::DataStorage* dataStorage);

    private:
        SR_NODISCARD SR_MATH_NS::FVector2 GetSizes() const;

        void OnDestroy() override;

        void TransformUI();

    };
}

#endif //SRENGINE_ANCHOR_H
