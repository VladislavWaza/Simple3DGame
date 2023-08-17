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
    static const int _mapWidth = 32;
    static const int _mapHeight = 32;
    static const int _blockSide = 8;

    Ui::Game *ui;
    QTimer _timer;
    double _x, _y, _angle;
    QList<char> _map;
    QPixmap _mapPixmap;
    bool _keyW, _keyS, _keyA, _keyD;

    double rayCast(double angle);
};

#endif // GAME_H
