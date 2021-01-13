//
// Created by Nikita on 13.01.2021.
//

#include "GUI/Canvas.h"

Framework::Canvas::Canvas(Scripting::Script* script) {
    script->AddUsePoint();
    this->m_script = script;
}

bool Framework::Canvas::Close() {
    Helper::Debug::Log("Canvas::Close() : close canvas...");

    if (m_script->IsDestroy())
        m_script->RemoveUsePoint();
    else{
        m_script->RemoveUsePoint();
        m_script->Close();
        m_script->Destroy();
    }

    return true;
}

bool Framework::Canvas::Free() {
    Helper::Debug::Info("Canvas::Free() : free canvas pointer...");
    delete this;
    return true;
}

void Framework::Canvas::Draw() {
    if (m_script->GetStatus() == Scripting::Script::Status::Compiled) {
        if (!m_isInit)
            this->Init();

        if (m_hasDraw)
            m_script->Call("Draw");
    }
}

bool Framework::Canvas::Init() {
    if (m_script->IsNeedInit())
        m_script->Init();
    if (m_script->IsNeedStart())
        m_script->Start();

    this->m_hasDraw = this->m_script->FunctionExists("Draw");

    this->m_isInit = true;

    return true;
}
