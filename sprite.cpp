#include "sprite.h"

Sprite::Sprite(double x, double y, int textureID)
    :_x(x), _y(y), _textureID(textureID)
{
}

QPointF Sprite::getPos()
{
    return QPointF(_x, _y);
}

double Sprite::getX()
{
    return _x;
}

double Sprite::getY()
{
    return _y;
}

int Sprite::getTextureID()
{
    return _textureID;
}

Enemy::Enemy(double x, double y, int textureID)
    :Sprite(x, y, textureID)
{
}
