#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_newGame_clicked()
{
    _game = new Game;

    //при закрытии Game, окно удалится, будет вызван ~Game и сработает сигнал destroyed
    _game->setAttribute(Qt::WA_DeleteOnClose);
    connect(_game, &Game::destroyed, this, &Widget::slotGameClosed);

    this->setVisible(false);
    _game->show();
}

void Widget::slotGameClosed()
{
    this->setVisible(true);
}

