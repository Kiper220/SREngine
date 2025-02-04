//
// Created by Nikita on 30.12.2020.
//

#ifndef GAMEENGINE_TIME_H
#define GAMEENGINE_TIME_H

#include <Utils/Debug.h>
#include <Utils/Common/Singleton.h>

namespace SR_HTYPES_NS {
    class SR_DLL_EXPORT Time : public Singleton<Time> {
        friend class Singleton<Time>;
    public:
        using Clock = std::chrono::high_resolution_clock;
        using Point = std::chrono::time_point<std::chrono::steady_clock>;

    public:
        void Update() {
            m_point = Clock::now();
        }

        SR_NODISCARD Point Now() const noexcept { return m_point; }
        SR_NODISCARD uint64_t Count() const noexcept { return m_point.time_since_epoch().count(); }
        SR_NODISCARD float_t FClock() const noexcept { return static_cast<float_t>(Count()) / CLOCKS_PER_SEC / CLOCKS_PER_SEC; }

    private:
        Point m_point;

    };
}

#endif //GAMEENGINE_TIME_H
