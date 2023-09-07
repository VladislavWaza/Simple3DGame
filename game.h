#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "sprite.h"

namespace Ui {
class Game;
}

class Game : public QWidget
{
    Q_OBJECT

public:
    explicit Game(QWidget *parent = nullptr);
    ~Game();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void updateInterface();

private:
    //Изменение данных констант может привести к некорректной работе программы!
    static const int _screenLabelSide = 512;
    static const int _mapLabelSide = 256;
    static const int _mapWidth = 32;
    static const int _mapHeight = 32;
    static const int _blockSide = 8;
    static const int _textureSide = 512;
    static constexpr double _fov = 60.0;
    static constexpr double _moveSpeed = 0.015;
    static constexpr double _rotationSpeed = 32;
    static constexpr double _playerCollisionRadius = 1;

    Ui::Game *ui;
    QTimer _timer;
    double _x, _y, _angle;
    QList<char> _map;
    QPixmap _mapPixmap;
    bool _keyW, _keyS, _keyA, _keyD;
    int _msOfLastFrame;
    int _frameTime;
    QList<QPixmap> _wallsTex;
    QList<QPixmap> _spritesTex;
    QList<Sprite> _sprites;
    QList<double> _distances;

    /* Возвращает расстояние до стены,
     * в texX помещает x координату точки на текстуре в которую пришел луч
     * в texNumber помещает номер текстуры стены
     */
    double rayCast(double angle, int *texX = nullptr, int *texNumber = nullptr);
    /* Перемещает игрока на расстояние dist по направлению угла angle
     * Учитвает коллизию игрока и не дает слишком близко подходить к стене
     */
    void movePlayer(double dist, double angle);
    /* Выполняет сорировку вставками по невозрастанию расстояние до спрайта */
    void sortSprites();
    void drawSprite(const Sprite& sprite, QPainter &painter);
};

#endif // GAME_H
