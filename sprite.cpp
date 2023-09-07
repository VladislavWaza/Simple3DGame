#include "sprite.h"

Sprite::Sprite(double x, double y, int textureID)
    :_x(x), _y(y), _textureID(textureID)
{
}

QPointF Sprite::getPos() const
{
    return QPointF(_x, _y);
}

double Sprite::getX() const
{
    return _x;
}

double Sprite::getY() const
{
    return _y;
}

int Sprite::getTextureID() const
{
    return _textureID;
}

Enemy::Enemy(double x, double y, int textureID)
    :Sprite(x, y, textureID)
{
}
