#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>

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
    QPixmap _wall1;

    double rayCast(double angle, int *texX = nullptr);
    void movePlayer(double dist, double angle);
};

#endif // GAME_H
