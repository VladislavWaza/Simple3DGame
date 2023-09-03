#ifndef SPRITE_H
#define SPRITE_H

#include <QPointF>

class Sprite
{
public:
    Sprite(double x, double y, int textureID);

    QPointF getPos();
    double getX();
    double getY();
    int getTextureID();

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
