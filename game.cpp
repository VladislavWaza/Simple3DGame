#include <QPainter>
#include <QTime>
#include "game.h"
#include "ui_game.h"

Game::Game(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Game)
{
    ui->setupUi(this);
    ui->screenLabel->setFixedSize(_screenLabelSide,_screenLabelSide);
    ui->mapLabel->setFixedSize(_mapLabelSide,_mapLabelSide);

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

    //хардкодим мобов
    _sprites.append(Sprite(30, 200, 1));
    _sprites.append(Sprite(100, 60, 2));
    _sprites.append(Sprite(12, 12, 2));


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
    _wallsTex.append(QPixmap(":/img/wall1.png"));
    _spritesTex.append(QPixmap(":/img/sprite1.png"));
    _spritesTex.append(QPixmap(":/img/sprite2.png"));

    //расширяем массив для хранения расстояний до стены
    _distances.resize(_screenLabelSide);

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
        if (curPos.x() <= _screenLabelSide && curPos.y() <= _screenLabelSide)
        {
            double dx = curPos.x() - _screenLabelSide / 2;
            dx = dx / _screenLabelSide + 0.5;
            dx = dx * dx * dx * (dx * (dx * 6 - 15) + 10) - 0.5;
            _angle += dx * _rotationSpeed;
            if (_angle >= 360)
                _angle = fmod(_angle, 360);
            if (_angle < 0)
                _angle = fmod(_angle, 360) + 360;
        }
    }
    if (this->isActiveWindow())
        QCursor::setPos(QWidget::mapToGlobal(ui->screenLabel->pos() + QPoint(_screenLabelSide / 2, _screenLabelSide / 2)));

    //отрисовка карты
    QPixmap pMap = _mapPixmap;
    QPainter painter(&pMap);
    //отрисовка положение игрока
    painter.setPen(QPen(QColor(0, 100, 0), 2));
    painter.drawPoint(_x,_y);
    //отрисовка луча видимости
    painter.setPen(QPen(QColor(150, 150, 0), 0.5));
    double dist = rayCast(_angle);
    painter.drawLine(_x, _y, _x + dist * cos(qDegreesToRadians(_angle)), _y + dist * sin(qDegreesToRadians(_angle)));
    //отрисовка мобов на карте
    painter.setPen(QPen(QColor(150, 0, 0), 2));
    for(int i = 0; i < _sprites.size(); ++i)
    {
        painter.drawPoint(_sprites[i].getPos());
    }
    //отображение полученного изображения
    painter.end();
    ui->mapLabel->setPixmap(pMap);

    //подготовка к отрисовке основного экрана
    QPixmap pScreen(ui->screenLabel->size());
    painter.begin(&pScreen);

    //раскраска неба и земли
    painter.fillRect(0,0,pScreen.width(), pScreen.height() / 2, QColor(136, 171, 170));
    painter.fillRect(0,pScreen.height() / 2,pScreen.width(), pScreen.height() / 2, QColor(163, 85, 57));

    //испускаем лучи покрывающие угол обзора
    double angleDiff = _fov / (_screenLabelSide - 1);
    double angle = _angle - _fov / 2;

    for (int i = 0; i < _screenLabelSide; ++i)
    {
        int texX, texNumber;
        dist = rayCast(angle, &texX, &texNumber);
        if (dist > 0) //если луч наткнулся на стену отрисовываем её
        {
            //запоминаем расстояние до стены
            _distances[i] = dist;
            //лучу соответствует полоска из пикселей текстуры стены
            QPixmap line = _wallsTex[texNumber - 1].copy(texX, 0, 1, _wallsTex[texNumber - 1].height());
            //какой будет итоговая высота полоски
            double columnHeight = _screenLabelSide/(dist*cos(qDegreesToRadians(angle-_angle))) * _blockSide;
            if (columnHeight > _screenLabelSide)
            {
                //если полоска больше экрана, то обрежем преждем чем растянем
                double stretchFactor = columnHeight / _screenLabelSide; //во сколько раз полоска больше экрана
                //отрезаем от полоски все что не будет отображено
                line = line.copy(0, (columnHeight / 2 - _screenLabelSide / 2) / stretchFactor,
                                 1, _screenLabelSide / stretchFactor);
                columnHeight = _screenLabelSide;
            }
            //растягиваем или сжимаем полоску до желаемой высоты
            if (columnHeight < line.height())
                line = line.scaled(1, columnHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            else
                line = line.scaled(1, columnHeight);
            //рисуем полоску
            painter.drawPixmap(i, (_screenLabelSide - columnHeight) / 2, line);
        }
        angle += angleDiff;
    }

    //сортировка спрайтов
    for (int i = 0; i < _sprites.size(); ++i)
    {
        int j = i - 1;
        Sprite key = _sprites[i];
        double keyDist = sqrt(pow(_x - key.getX(), 2) + pow(_y - key.getY(), 2));
        while (j >= 0 && sqrt(pow(_x - _sprites[j].getX(), 2) + pow(_y - _sprites[j].getY(), 2)) < keyDist)
        {
            _sprites[j + 1] = _sprites[j];
            j -= 1;
        }
        _sprites[j + 1] = key;
    }


    //отрисовка спрайтов
    for(int i = 0; i < _sprites.size(); ++i)
    {
        double spriteAngle = qRadiansToDegrees(atan2(_sprites[i].getY() - _y, _sprites[i].getX() - _x));
        while (spriteAngle - _angle >  180) spriteAngle -= 360;
        while (spriteAngle - _angle < -180) spriteAngle += 360;

        if (fabs(spriteAngle - _angle) < _fov/2 + 10)
        {
            double spriteDist = sqrt(pow(_x - _sprites[i].getX(), 2) + pow(_y - _sprites[i].getY(), 2));
            QPixmap sprite = _spritesTex[_sprites[i].getTextureID() - 1];
            if (spriteDist != 0)
            {
                double spriteWidth = sprite.width() * 8.0 / spriteDist;
                double spriteHeight = sprite.height() * 8.0 / spriteDist;
                int x = ((spriteAngle - _angle)/_fov + 0.5) * _screenLabelSide - spriteWidth / 2;
                int y = _screenLabelSide / 2 - spriteHeight / 2;
                QRect spriteRect(0,0,spriteWidth,spriteHeight);
                QRect screenRect(-x,-y,_screenLabelSide,_screenLabelSide);
                spriteRect = spriteRect.intersected(screenRect);
                sprite = sprite.copy(spriteRect.x() / (spriteWidth / _screenLabelSide),
                                     spriteRect.y() / (spriteHeight / _screenLabelSide),
                                     spriteRect.width() / (spriteWidth / _screenLabelSide),
                                     spriteRect.height() / (spriteHeight / _screenLabelSide));
                if (spriteWidth > _screenLabelSide || spriteHeight >= _screenLabelSide)
                {
                    if (spriteWidth > spriteHeight)
                        sprite = sprite.scaledToWidth(_screenLabelSide);
                    else
                        sprite = sprite.scaledToHeight(_screenLabelSide);
                }
                else
                {
                    spriteWidth = sprite.width() * 8 / spriteDist;
                    spriteHeight = sprite.height() * 8 / spriteDist;
                    sprite = sprite.scaled(spriteWidth, spriteHeight);
                }
                for (int j = 0; j < sprite.width(); ++j)
                {
                    if (spriteRect.x() + x + j >= 0 && spriteRect.x() + x + j < _screenLabelSide
                            && spriteDist < _distances[spriteRect.x() + x + j])
                    {
                        QPixmap line = sprite.copy(j, 0, 1, sprite.height());
                        painter.drawPixmap(spriteRect.x() + x + j, spriteRect.y() + y, line);
                    }
                }
            }
        }
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

double Game::rayCast(double angle, int *texX, int *texNumber)
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
        {
            hit = true;
            if (texNumber != nullptr)
                *texNumber = _map[mapX * _mapHeight + mapY];
        }
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

    //если до стены по направлению движения еще далеко, то перемещаемся
    if (fabs((distToWall - dist) * cos(qDegreesToRadians(angle))) > _playerCollisionRadius)
        _x += dist * cos(qDegreesToRadians(angle));
    if (fabs((distToWall - dist) * sin(qDegreesToRadians(angle))) > _playerCollisionRadius)
        _y += dist * sin(qDegreesToRadians(angle));

    //проверяем не подошли ли мы к какой-нибудь стене() слишком близко
    //если это так немного отходим назад
    //это действие добавляе коллизию персонажу игрока
    //и избавляет от случаев когда двигаясь к далекой стене игрок проходит вплотную с другой стеной
    if (rayCast(0) < _playerCollisionRadius)
    {
        _x -= _playerCollisionRadius - rayCast(0);
    }
    if (rayCast(180) < _playerCollisionRadius)
    {
        _x += _playerCollisionRadius - rayCast(180);
    }

    if (rayCast(90) < _playerCollisionRadius)
    {
        _y -= _playerCollisionRadius - rayCast(90);
    }
    if (rayCast(270) < _playerCollisionRadius)
    {
        _y += _playerCollisionRadius - rayCast(270);
    }
}
