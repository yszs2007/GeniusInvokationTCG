#ifndef CARDWIDGET_H
#define CARDWIDGET_H

#include <QWidget>
#include "Card.h"

class CardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CardWidget(Card *card, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    Card *m_card;
};

#endif