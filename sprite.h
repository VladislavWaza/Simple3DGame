#ifndef SPRITE_H
#define SPRITE_H

#include <QPointF>

class Sprite
{
public:
    Sprite(double x, double y, int textureID);

    QPointF getPos() const;
    double getX() const;
    double getY() const;
    int getTextureID() const;

private:
    double _x, _y;
    int _textureID;
};


class Enemy : public Sprite
{
//добавить сюда поле маршрут
public:
   Enemy(double x, double y, int textureID);
};

#endif // SPRITE_H
