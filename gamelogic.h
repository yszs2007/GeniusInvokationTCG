#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "card.h"
#include <QList>
#include <QVector>

class GameLogic
{
public:
    GameLogic() = default;

    void initActionCardDeck();
    void drawInitialHand();
    void mulligan(const QVector<int>& indexes);
    void drawCard();

    QList<ActionCard*> actionDeck;
    QList<ActionCard*> playerHand;
    bool canMulligan = true;
};

#endif // GAMELOGIC_H