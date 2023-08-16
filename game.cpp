#include <QPainter>
#include "game.h"
#include "ui_game.h"

Game::Game(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Game)
{
    ui->setupUi(this);
    ui->screenLabel->setFixedSize(512,512);
    ui->mapLabel->setFixedSize(256,256);
    setMouseTracking(true);
    ui->screenLabel->setMouseTracking(true);

    _x = 20;
    _y = 20;
    _angle = 45;
    _map.resize(_mapHeight * _mapWidth);
    _keyW = false;
    _keyS = false;
    _keyA = false;
    _keyD = false;

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
                painter.fillRect(x * _blockWidth, y * _blockHeight, _blockWidth, _blockHeight, QColor(77,34,14));
            }
        }
    }
    painter.end();
    _mapPixmap = pm;

    connect(&_timer, &QTimer::timeout, this, &Game::updateInterface);
    _timer.start(20);
}

Game::~Game()
{
    delete ui;
}

void Game::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W)
    {
        _x += 0.5 * cos(qDegreesToRadians(_angle));
        _y += 0.5 * sin(qDegreesToRadians(_angle));
        _keyW = true;
    }
    if (event->key() == Qt::Key_S)
    {
        _x -= 0.5 * cos(qDegreesToRadians(_angle));
        _y -= 0.5 * sin(qDegreesToRadians(_angle));
        _keyS = true;
    }
    if (event->key() == Qt::Key_A)
    {
        _x += 0.5 * cos(qDegreesToRadians(_angle - 90));
        _y += 0.5 * sin(qDegreesToRadians(_angle - 90));
        _keyA = true;
    }
    if (event->key() == Qt::Key_D)
    {
        _x -= 0.5 * cos(qDegreesToRadians(_angle - 90));
        _y -= 0.5 * sin(qDegreesToRadians(_angle - 90));
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

void Game::mouseMoveEvent(QMouseEvent *event)
{
    QPoint curPos = event->pos() - ui->screenLabel->pos();
    if (curPos.x() >= 0 && curPos.y() >= 0)
        if (curPos.x() <= ui->screenLabel->width() && curPos.y() <= ui->screenLabel->height())
        {
            float dx = curPos.x() - ui->screenLabel->width() / 2;
            dx = dx / ui->screenLabel->width() + 0.5;
            dx = dx * dx * dx * (dx * (dx * 6 - 15) + 10) - 0.5;
            _angle += dx * 5;
        }
    QWidget::mouseMoveEvent(event);
}

void Game::updateInterface()
{
    QPixmap pMap = _mapPixmap;
    QPainter painter(&pMap);
    painter.setPen(QPen(QColor(0, 100, 0), 2));
    if (_keyW)
    {
        _x += 0.5 * cos(qDegreesToRadians(_angle));
        _y += 0.5 * sin(qDegreesToRadians(_angle));
    }
    if (_keyS)
    {
        _x -= 0.5 * cos(qDegreesToRadians(_angle));
        _y -= 0.5 * sin(qDegreesToRadians(_angle));
    }
    if (_keyA)
    {
        _x += 0.5 * cos(qDegreesToRadians(_angle - 90));
        _y += 0.5 * sin(qDegreesToRadians(_angle - 90));
    }
    if (_keyD)
    {
        _x -= 0.5 * cos(qDegreesToRadians(_angle - 90));
        _y -= 0.5 * sin(qDegreesToRadians(_angle - 90));
    }
    painter.drawPoint(_x,_y);
    painter.setPen(QPen(QColor(150, 150, 0), 0.5));
    float dist = getDistToWall(512, _angle);
    if (dist > 0)
    {
        painter.drawLine(_x, _y, _x + dist * cos(qDegreesToRadians(_angle)), _y + dist * sin(qDegreesToRadians(_angle)));
    }
    painter.end();
    ui->mapLabel->setPixmap(pMap);


    QPixmap pScreen(ui->screenLabel->size());
    pScreen.fill();
    painter.begin(&pScreen);
    painter.setPen(QPen(QColor(77,34,14), 1));

    float angleDiff = 90.0 / (ui->screenLabel->height() - 1);
    float angle = _angle - 45;
    for (int i = 0; i < ui->screenLabel->height(); ++i)
    {
        dist = getDistToWall(64, angle);
        if (dist > 0)
        {
            float columnHeight = ui->screenLabel->height()/(dist*cos(qDegreesToRadians(angle-_angle)));
            painter.drawLine(i, (ui->screenLabel->height() - columnHeight) / 2,
                             i, ui->screenLabel->height() - (ui->screenLabel->height() - columnHeight) / 2);
        }
        angle += angleDiff;
    }

    painter.end();
    ui->screenLabel->setPixmap(pScreen);
}

float Game::getDistToWall(float range, float angle)
{
    float dist = 0;
    bool isWall = false;
    while (dist < range)
    {
        float x = _x + dist * cos(qDegreesToRadians(angle));
        float y = _y + dist * sin(qDegreesToRadians(angle));
        if (_map[static_cast<int>(x / _blockWidth) * _mapHeight + static_cast<int>(y / _blockHeight)] != 0)
        {
            isWall = true;
            break;
        }
        dist += 0.1;
    }
    if (isWall)
        return dist;
    else
        return -1;
}
