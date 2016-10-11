#pragma once

namespace Engine {
namespace Math {

class MADGINE_EXPORT Bounds
{
public:
    Bounds(float left = 0.f, float bottom = 0.f, float right = 0.f, float top = 0.f);
    Bounds(const Ogre::Vector3 &min, const Ogre::Vector3 &max);

    float left() const;

    float top() const;

    float right() const;

    float bottom() const;

    float width() const;

    float height() const;

    Ogre::Vector2 size() const;

	Ogre::Vector2 bottomLeft() const;

    std::array<Ogre::Vector2, 4> corners() const;

	Ogre::Vector2 operator *(const Ogre::Vector2 &v);

private:
    float mLeft, mTop, mRight, mBottom;
};

} // namespace Math
} // namespace Core

