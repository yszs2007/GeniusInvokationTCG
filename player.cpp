#include "player.h"

Player::Player(QString name) : m_name(name) {}
QString Player::name() const { return m_name; }
void Player::addRole(RoleCard *role) { m_roles << role; }
RoleCard* Player::getActiveRole() const { return m_roles.first(); }
QVector<Card*> Player::getHandCards() const { return m_handCards; }