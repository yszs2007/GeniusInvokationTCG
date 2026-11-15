#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QVector>
#include "Card.h"

class Player
{
public:
    Player(QString name);
    QString name() const;
    void addRole(RoleCard *role);
    RoleCard* getActiveRole() const;
    QVector<Card*> getHandCards() const;

private:
    QString m_name;
    QVector<RoleCard*> m_roles;
    QVector<Card*> m_handCards;
};

#endif
