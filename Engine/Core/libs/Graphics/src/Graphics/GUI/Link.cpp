//
// Created by Monika on 18.01.2022.
//

#include <Graphics/GUI/Link.h>
#include <Graphics/GUI/Pin.h>

#include <imgui-node-editor/imgui_node_editor.h>

namespace SR_GRAPH_NS::GUI {
    uintptr_t Link::GetId() const {
        return reinterpret_cast<uintptr_t>(this);
    }

    void Link::Draw() const {
        ax::NodeEditor::Link(GetId(), m_startPin->GetId(), m_endPin->GetId());
    }

    Link::Link()
        : Link(nullptr, nullptr, ImColor(255, 255, 255, 255))
    { }

    Link::Link(Pin* start, Pin* end)
        : Link(start, end, ImColor(255, 255, 255, 255))
    { }

    Link::Link(Pin* start, Pin* end, ImColor color)
        : m_startPin(start)
        , m_endPin(end)
        , m_color(color)
    {
        if (m_startPin)
            m_startPin->AddLink(this);

        if (m_endPin)
            m_endPin->AddLink(this);
    }

    Link::~Link() {
        if (m_startPin) {
            m_startPin->RemoveLink(this);
            m_startPin = nullptr;
        }

        if (m_endPin) {
            m_endPin->RemoveLink(this);
            m_endPin = nullptr;
        }
    }

    bool Link::IsLinked(Pin* pPin) const {
        return pPin && (m_startPin == pPin || m_endPin == pPin);
    }
}