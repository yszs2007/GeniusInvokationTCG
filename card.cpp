#include "Card.h"

// 卡牌基类
Card::Card(QString name, int cost, CardType type, Element element)
    : m_name(name), m_cost(cost), m_type(type), m_element(element)
{}

QString Card::getName() const { return m_name; }
int Card::getCost() const { return m_cost; }
CardType Card::getType() const { return m_type; }
Element Card::getElement() const { return m_element; }

// -----------------------------------------------------------------------------
// 角色卡
// 构造函数增加了 RoleID
RoleCard::RoleCard(QString name, int hp, int atk, Element element, RoleID roleId)
    : Card(name, 0, CARD_ROLE, element),
    m_maxHp(hp), m_hp(hp), m_atk(atk), m_roleId(roleId)
{}

void RoleCard::useCard() {}

void RoleCard::attack(RoleCard* target) {
    if (target) {
        target->setHp(target->getHp() - this->m_atk);
    }
}

int RoleCard::getHp() const { return m_hp; }
int RoleCard::getMaxHp() const { return m_maxHp; }
int RoleCard::getAtk() const { return m_atk; }
void RoleCard::setHp(int hp) { m_hp = hp; }

// 获取角色ID
RoleID RoleCard::getRoleId() const {
    return m_roleId;
}

// -----------------------------------------------------------------------------
// 行动卡
ActionCard::ActionCard(QString name, int cost, Element element, QString effect)
    : Card(name, cost, CARD_ACTION, element), m_effect(effect)
{}

void ActionCard::useCard() {}
QString ActionCard::getEffect() const { return m_effect; }