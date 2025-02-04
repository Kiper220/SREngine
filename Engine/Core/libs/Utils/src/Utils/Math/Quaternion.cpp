//
// Created by Nikita on 01.03.2021.
//

#include <Utils/Math/Quaternion.h>
#include <Utils/Math/Vector3.h>
#include <Utils/Math/Matrix3x3.h>
#include <Utils/Math/Matrix4x4.h>

namespace SR_MATH_NS {
    Vector3<Unit> Quaternion::EulerAngle() const {
        return Vector3<Unit>(glm::eulerAngles(glm::normalize(self))).Degrees();
    }

    Quaternion::Quaternion(const Vector3<Unit>& eulerAngle) {
        Vector3<T> c = (eulerAngle * T(0.5)).Cos();
        Vector3<T> s = (eulerAngle * T(0.5)).Sin();

        this->w = c.x * c.y * c.z + s.x * s.y * s.z;
        this->x = s.x * c.y * c.z - c.x * s.y * s.z;
        this->y = c.x * s.y * c.z + s.x * c.y * s.z;
        this->z = c.x * c.y * s.z - s.x * s.y * c.z;

        ///self = p_euler.ToGLM();
    }

    Vector3<Unit> Quaternion::operator*(const Vector3<Unit> &v) const noexcept {
        Vector3<Unit> const QuatVector(x, y, z);
        Vector3<Unit> const uv = QuatVector.Cross(v);
        Vector3<Unit> const uuv = QuatVector.Cross(uv);

        return v + ((uv * w) + uuv) * static_cast<Unit>(2);
    }

    Quaternion Quaternion::Rotate(const Vector3<Unit> &v) const {
        if (v.Empty())
            return *this;

        glm::quat q = glm::rotate(self, 1.f, glm::radians(glm::vec3(v.x, v.y, v.z)));
        return Quaternion(q);
    }

    Matrix4x4 Quaternion::ToMat4x4() const {
        return Matrix4x4(mat4_cast(self));
    }

   Vector3<Unit> Quaternion::operator/(const Vector3<Unit> &v) const {
        glm::vec3 rot = EulerAngle().ToGLM();

        /// TODO: здесь должна быть инвертирована ось z?
        glm::fquat q = glm::vec3(1) / glm::vec3(
                rot.x,
                rot.y,
                -rot.z
        );

        return Vector3<Unit>(q * v.ToGLM());
    }

    Quaternion Quaternion::FromEuler(const Vector3 <Unit> &euler) {
        return euler.Radians().ToQuat();
    }
}