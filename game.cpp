#include <QPainter>
#include <QTime>
#include "game.h"
#include "ui_game.h"

Game::Game(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Game)
{
    ui->setupUi(this);
    ui->screenLabel->setFixedSize(512,512);
    ui->mapLabel->setFixedSize(256,256);

    //QApplication::setOverrideCursor(Qt::BlankCursor);
    ui->screenLabel->setCursor(Qt::BlankCursor);

    _x = 20;
    _y = 20;
    _angle = 45;
    _keyW = false;
    _keyS = false;
    _keyA = false;
    _keyD = false;

    _map.resize(_mapHeight * _mapWidth);
    //делаем стены на границах
    for (int y = 0; y < _mapHeight; ++y)
    {
        _map[y] = 1;
        _map[(_mapWidth - 1) * _mapHeight + y] = 1;
    }
    for (int x = 0; x < _mapWidth; ++x)
    {
        _map[x * _mapHeight] = 1;
        _map[x * _mapHeight + (_mapHeight - 1)] = 1;
    }

    //хардкодим карту
    _map[8 * _mapHeight + 9] = 1;
    _map[9 * _mapHeight + 8] = 1;
    _map[10 * _mapHeight + 10] = 1;
    _map[10 * _mapHeight + 11] = 1;
    _map[10 * _mapHeight + 12] = 1;
    _map[10 * _mapHeight + 13] = 1;
    _map[8 * _mapHeight + 13] = 1;
    _map[9 * _mapHeight + 15] = 1;
    _map[8 * _mapHeight + 15] = 1;
    _map[10 * _mapHeight + 15] = 1;

    //отрисовываем карту
    QPixmap pm(ui->mapLabel->size());
    pm.fill(QColor(244, 164, 96));
    QPainter painter(&pm);
    for (int x = 0; x < _mapWidth; ++x)
    {
        for (int y = 0; y < _mapHeight; ++y)
        {
            if (_map[x * _mapHeight + y] == 1)
            {
                painter.fillRect(x * _blockSide, y * _blockSide, _blockSide, _blockSide, QColor(77,34,14));
            }
        }
    }
    painter.end();
    _mapPixmap = pm;

    _frameTime = 0;
    _msOfLastFrame = QTime::currentTime().msecsSinceStartOfDay();

    //запускаем процесс
    connect(&_timer, &QTimer::timeout, this, &Game::updateInterface);
    _timer.start(1);
}

Game::~Game()
{
    delete ui;
}

void Game::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W)
    {
        _keyW = true;
    }
    if (event->key() == Qt::Key_S)
    {
        _keyS = true;
    }
    if (event->key() == Qt::Key_A)
    {
        _keyA = true;
    }
    if (event->key() == Qt::Key_D)
    {
        _keyD = true;
    }
    QWidget::keyPressEvent(event);
}

void Game::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W)
    {
        _keyW = false;
    }
    if (event->key() == Qt::Key_S)
    {
        _keyS = false;
    }
    if (event->key() == Qt::Key_A)
    {
        _keyA = false;
    }
    if (event->key() == Qt::Key_D)
    {
        _keyD = false;
    }
    QWidget::keyPressEvent(event);
}

void Game::updateInterface()
{
    //перемещение
    if (_keyW)
    {
        _x += _moveSpeed * _frameTime * cos(qDegreesToRadians(_angle));
        _y += _moveSpeed * _frameTime * sin(qDegreesToRadians(_angle));
    }
    if (_keyS)
    {
        _x -= _moveSpeed * _frameTime * cos(qDegreesToRadians(_angle));
        _y -= _moveSpeed * _frameTime * sin(qDegreesToRadians(_angle));
    }
    if (_keyA)
    {
        _x += _moveSpeed * _frameTime * cos(qDegreesToRadians(_angle - 90));
        _y += _moveSpeed * _frameTime * sin(qDegreesToRadians(_angle - 90));
    }
    if (_keyD)
    {
        _x -= _moveSpeed * _frameTime * cos(qDegreesToRadians(_angle - 90));
        _y -= _moveSpeed * _frameTime * sin(qDegreesToRadians(_angle - 90));
    }

    //поворот
    QPoint curPos = QWidget::mapFromGlobal(QCursor::pos()) - ui->screenLabel->pos();
    if (curPos.x() >= 0 && curPos.y() >= 0)
    {
        if (curPos.x() <= ui->screenLabel->width() && curPos.y() <= ui->screenLabel->height())
        {
            double dx = curPos.x() - ui->screenLabel->width() / 2;
            dx = dx / ui->screenLabel->width() + 0.5;
            dx = dx * dx * dx * (dx * (dx * 6 - 15) + 10) - 0.5;
            _angle += dx * _rotationSpeed;
        }
    }
    if (this->isActiveWindow())
        QCursor::setPos(QWidget::mapToGlobal(ui->screenLabel->pos() + QPoint(256, 256)));

    //отрисовка игрока на карте и луча
    QPixmap pMap = _mapPixmap;
    QPainter painter(&pMap);
    painter.setPen(QPen(QColor(0, 100, 0), 2));
    painter.drawPoint(_x,_y);
    painter.setPen(QPen(QColor(150, 150, 0), 0.5));
    double dist = rayCast(_angle);
    painter.drawLine(_x, _y, _x + dist * cos(qDegreesToRadians(_angle)), _y + dist * sin(qDegreesToRadians(_angle)));
    painter.end();
    ui->mapLabel->setPixmap(pMap);


    QPixmap pScreen(ui->screenLabel->size());
    pScreen.fill();
    painter.begin(&pScreen);
    painter.setPen(QPen(QColor(77,34,14), 1));

    double angleDiff = _fov / (ui->screenLabel->height() - 1);
    double angle = _angle - _fov / 2;
    for (int i = 0; i < ui->screenLabel->height(); ++i)
    {
        dist = rayCast(angle);
        if (dist > 0)
        {
            double columnHeight = ui->screenLabel->height()/(dist*cos(qDegreesToRadians(angle-_angle))) * 2;
            if (columnHeight > ui->screenLabel->height())
                columnHeight = ui->screenLabel->height();
            painter.drawLine(i, (ui->screenLabel->height() - columnHeight) / 2,
                             i, (ui->screenLabel->height() + columnHeight) / 2);
        }
        angle += angleDiff;
    }

    painter.end();
    ui->screenLabel->setPixmap(pScreen);

    _frameTime = QTime::currentTime().msecsSinceStartOfDay() - _msOfLastFrame;
    _msOfLastFrame = QTime::currentTime().msecsSinceStartOfDay();
    qDebug() << _frameTime;
}

double Game::rayCast(double angle)
{
    //положение игрока в координатах карты
    double posX = _x / _blockSide;
    double posY = _y / _blockSide;

    //координаты клетки, в которой находится игрок
    int mapX = static_cast<int>(posX);
    int mapY = static_cast<int>(posY);

    //вектор луча
    double rayDirX = cos(qDegreesToRadians(angle));
    double rayDirY = sin(qDegreesToRadians(angle));

    //длина луча от одной стороны x или y до следующей стороны x или y
    double deltaDistX = 1e30;
    if (rayDirX != 0)
        deltaDistX = abs(1 / rayDirX);
    double deltaDistY = 1e30;
    if (rayDirY != 0)
        deltaDistY = abs(1 / rayDirY);

    //расстояние до стены
    double perpWallDist;

    //в каком направлении двигаться в направлении x или y (либо +1, либо -1)
    int stepX;
    int stepY;

    bool hit = false; //был ли удар луча о стену?
    char side; //удар был о горизонатльную или вертикальную стену?

    //длина луча от текущего положения до следующей стороны x или y
    double sideDistX;
    double sideDistY;
    if (rayDirX < 0)
    {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
    }
    if (rayDirY < 0)
    {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
    }

    while (!hit)
    {
        //переход к следующему квадрату карты либо в направлении x, либо в направлении y
        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        }
        else
        {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }
        //Проверка, не ударился ли луч о стену
        if (_map[mapX * _mapHeight + mapY] > 0)
            hit = true;
    }
    if(side == 0)
        perpWallDist = (sideDistX - deltaDistX);
    else
        perpWallDist = (sideDistY - deltaDistY);
    return perpWallDist * _blockSide;
}
