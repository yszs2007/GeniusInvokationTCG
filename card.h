#ifndef CARD_H
#define CARD_H

#include <QString>

// 元素类型
enum Element {
    Anemo,   // 风
    Geo,     // 岩
    Electro, // 雷
    Dendro,  // 草
    Hydro,   // 水
    Pyro,    // 火
    Cryo,    // 冰
    NoElement
};

// 卡牌类型
enum CardType {
    CARD_ROLE,
    CARD_ACTION,
    CARD_TALENT,
    CARD_WEAPON,
    CARD_ARTIFACT
};

// ===================== 在这里加入：角色类型枚举 =====================
// 14个角色枚举（对应你要做的角色）
enum RoleID {
    Role_Babala,        // 芭芭拉
    Role_Bannite,       // 班尼特
    Role_Chongyun,      // 重云
    Role_Diluke,        // 迪卢克
    Role_Feixieer,      // 菲谢尔
    Role_Kaiya,         // 凯亚
    Role_Kawei,         // 卡维
    Role_Kelai,         // 柯莱
    Role_Keqing,        // 刻晴
    Role_Ningguang,     // 凝光
    Role_ShanHaiGong,   // 珊瑚宫心海
    Role_ShaTang,       // 砂糖
    Role_Wendi,         // 温迪
    Role_Zhongli        // 钟离
};
// ==================================================================

class Card
{
public:
    Card(QString name, int cost, CardType type, Element element);
    virtual ~Card() = default;

    virtual void useCard() = 0;

    QString getName() const;
    int getCost() const;
    CardType getType() const;
    Element getElement() const;

protected:
    QString m_name;
    int m_cost;
    CardType m_type;
    Element m_element;
};

// 角色卡
class RoleCard : public Card
{
public:
    RoleCard(QString name, int hp, int atk, Element element, RoleID roleId);

    void useCard() override;
    void attack(RoleCard* target);

    int getHp() const;
    int getMaxHp() const;
    int getAtk() const;
    void setHp(int hp);

    // 获取角色ID（用来显示图片、放技能）
    RoleID getRoleId() const;

private:
    int m_maxHp;
    int m_hp;
    int m_atk;
    RoleID m_roleId; // 保存当前角色是哪一个
};

// 行动卡
class ActionCard : public Card
{
public:
    ActionCard(QString name, int cost, Element element, QString effect);

    void useCard() override;
    QString getEffect() const;

private:
    QString m_effect;
};

#endif // CARD_H