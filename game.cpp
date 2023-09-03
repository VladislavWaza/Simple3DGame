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

    //скрываем курсор
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

    //загружаем текстуры
    _wall1.load(":/img/wall1.png");

    //необходимые стартовые значения для счетчика кадров
    _frameTime = 0;
    _msOfLastFrame = QTime::currentTime().msecsSinceStartOfDay();

    //запускаем процесс игры
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
        movePlayer(_moveSpeed * _frameTime, _angle);
    if (_keyS)
        movePlayer(_moveSpeed * _frameTime, _angle + 180);
    if (_keyA)
        movePlayer(_moveSpeed * _frameTime, _angle - 90);
    if (_keyD)
        movePlayer(_moveSpeed * _frameTime, _angle + 90);

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
            if (_angle >= 360)
                _angle = fmod(_angle, 360);
            if (_angle < 0)
                _angle = fmod(_angle, 360) + 360;
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

    //подготовка к отрисовке основного экрана
    QPixmap pScreen(ui->screenLabel->size());
    painter.begin(&pScreen);

    //раскраска неба и земли
    painter.fillRect(0,0,pScreen.width(), pScreen.height() / 2, QColor(136, 171, 170));
    painter.fillRect(0,pScreen.height() / 2,pScreen.width(), pScreen.height() / 2, QColor(163, 85, 57));

    //испускаем лучи покрывающие угол обзора
    double angleDiff = _fov / (ui->screenLabel->width() - 1);
    double angle = _angle - _fov / 2;

    for (int i = 0; i < ui->screenLabel->width(); ++i)
    {
        int tex;
        dist = rayCast(angle, &tex);
        if (dist > 0) //если луч наткнулся на стену отрисовываем её
        {
            //лучу соответствует полоска из пикселей текстуры стены
            QPixmap line = _wall1.copy(tex, 0, 1, _wall1.height());
            //какой будет итоговая высота полоски
            double columnHeight = ui->screenLabel->height()/(dist*cos(qDegreesToRadians(angle-_angle))) * _blockSide;
            if (columnHeight > ui->screenLabel->height())
            {
                //если полоска больше экрана, то обрежем преждем чем растянем
                double stretchFactor = columnHeight / ui->screenLabel->height(); //во сколько раз полоска больше экрана
                //отрезаем от полоски все что не будет отображено
                line = line.copy(0, (columnHeight / 2 - ui->screenLabel->height() / 2) / stretchFactor,
                                 1, ui->screenLabel->height() / stretchFactor);
                columnHeight = ui->screenLabel->height();
            }
            //растягиваем или сжимаем полоску до желаемой высоты
            line = line.scaled(1, columnHeight);
            //рисуем полоску
            painter.drawPixmap(i, (ui->screenLabel->height() - columnHeight) / 2, line);
        }
        angle += angleDiff;
    }

    //отрисовка fps счетчика
    painter.setPen(QPen(QColor(0, 0, 0), 1));
    int fps = 1000;
    if (_frameTime != 0)
        fps /= _frameTime;
    painter.drawText(5,10, QString::number(fps));

    painter.end();
    ui->screenLabel->setPixmap(pScreen);

    //обновление информации о времени отрисовки кадра для расчета fps
    _frameTime = QTime::currentTime().msecsSinceStartOfDay() - _msOfLastFrame;
    _msOfLastFrame = QTime::currentTime().msecsSinceStartOfDay();
}

double Game::rayCast(double angle, int *texX)
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

    if (texX != nullptr)
    {
        double wallX; //где именно был нанесен удар лучом по стене
        if (side == 0)
            wallX = posY + perpWallDist * rayDirY;
        else
            wallX = posX + perpWallDist * rayDirX;
        wallX = fmod(wallX, 1); //оставлем дробную часть

        //вычисляем x координату в текстуре
        *texX = static_cast<int>(wallX * static_cast<double>(_textureSide));
        if(side == 0 && rayDirX > 0) *texX = _textureSide - *texX - 1;
        if(side == 1 && rayDirY < 0) *texX = _textureSide - *texX - 1;
    }
    return perpWallDist * _blockSide;
}

void Game::movePlayer(double dist, double angle)
{
    double distToWall = rayCast(angle);
    if (distToWall > dist) //если до стены еще далеко
    {
        //идем куда хотели
        _x += dist * cos(qDegreesToRadians(angle));
        _y += dist * sin(qDegreesToRadians(angle));
    }
    else //если до стены ближе чем желаем пройти
    {
        //подходим вплотную к стене
        _x += (distToWall - 0.1) * cos(qDegreesToRadians(angle));
        _y += (distToWall - 0.1) * sin(qDegreesToRadians(angle));

        //сохраняем остаток непройденного пути
        dist = dist - distToWall;
        //вычисляем ближайшее к желаемому направление и при этом параллельное стене
        if (abs(angle) < abs(angle - 90) && abs(angle) <= abs(angle - 180) && abs(angle) <= abs(angle - 270))
            angle = 0;
        if (abs(angle - 90) < abs(angle) && abs(angle - 90) <= abs(angle - 180) && abs(angle - 90) <= abs(angle - 270))
            angle = 90;
        if (abs(angle - 180) < abs(angle) && abs(angle - 180) <= abs(angle - 90) && abs(angle - 180) <= abs(angle - 270))
            angle = 180;
        if (abs(angle - 270) < abs(angle) && abs(angle - 270) <= abs(angle - 90) && abs(angle - 270) <= abs(angle - 180))
            angle = 270;

        //допроходим остаток непройденного пути в направлении параллельном стене
        distToWall = rayCast(angle);
        if (distToWall > dist)
        {
            _x += dist * cos(qDegreesToRadians(angle));
            _y += dist * sin(qDegreesToRadians(angle));
        }
        else
        {
            _x += (distToWall - 0.1) * cos(qDegreesToRadians(angle));
            _y += (distToWall - 0.1) * sin(qDegreesToRadians(angle));
        }
    }
}
