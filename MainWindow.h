#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QEvent>
#include <QStringList>
#include "card.h"
#include "gamelogic.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class ElementType {
    Ice,
    Grass,
    Wind,
    Fire,
    Thunder,
    Water,
    Any,
    Rock
};

// 技能类型：普通攻击 / 元素战技 / 元素爆发
enum class SkillType {
    None,
    A,
    E,
    Q
};

// ===================== 【严格按你提供】召唤物类型 =====================
enum class SummonType {
    None,
    StormEye,      // 温迪Q - 暴风之眼
    LargeWindSpirit,// 砂糖Q - 大型风灵
    StoneStele,     // 钟离E - 岩脊
    JadeScreen,     // 凝光E - 璇玑屏
    Oz,             // 菲谢尔E - 奥兹
    CuileinAnbar,   // 柯莱Q - 柯里安巴
    SeaHorses       // 心海E - 化海月
};

// ===================== 持续效果牌（西风大教堂 / 望舒客栈） =====================
enum class StatusEffectType {
    None,
    WestCathedral,  // 西风大教堂
    WangShuInn      // 望舒客栈
};

// ===================== 放置区统一条目（召唤物 + 持续牌） =====================
struct PlacedEntry
{
    enum { Summon, Status } type;
    SummonType summon;
    StatusEffectType status;
    int remainingTurns;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setPlayerTeam(const QVector<RoleID>& team);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onMulliganBtnClicked();
    void rollDice();
    void reRollSelectedDice();
    void arrangeDiceToRightSide();
    void showToolTip(int index);
    void hideToolTip();
    void onPlayCardClicked();
    void onSwitchRoleClicked();
    void onEndTurnClicked();
    void startNewBigRound();

    void doEnemyOneAction();
    void doEnemyFullAction();

    // ==================== 技能系统槽函数 ====================
    void onSkillBtnClicked(SkillType type);   // A/E/Q按钮点击
    void onCastSkillClicked();               // 释放技能按钮点击
    void refreshSkillButtonState();          // 刷新按钮可用状态

    // 重新开始游戏
    void onRestartGameClicked();

private:
    void initHandUI();
    void refreshHandUI();
    void selectCard(int index);
    ElementType getRoleElementType(RoleID id);
    QString getCardDesc(const QString& name);
    QString getRoleName(RoleID id);

    QString getRoleElementName(RoleID id);
    QString getRoleWeaponName(RoleID id);

    enum class WeaponType {
        Sword,
        Claymore,
        Polearm,
        Bow,
        Catalyst,
        Unknown
    };
    WeaponType getRoleWeaponType(RoleID id);

    void createRoleStatusPanel();
    void updateRoleStatus();
    void updateRoleStatusHighlight(int activeIndex);
    void createEnemyCharacter();

    enum CardCostType {
        Cost_None,
        Cost_Any1,
        Cost_Any2,
        Cost_Same2
    };
    CardCostType getCardCostType(const QString& name);
    bool checkDiceMatchCost();
    void selectCardForPlay(int index);
    void selectDiceForPlay(int index);

    // 元素反应系统
    enum class ReactionType {
        None,
        Vaporize,
        Melt,
        Freeze,
        Electro,
        Quicken,
        Bloom,
        Swirl,
        Crystallize
    };

    ReactionType calculateReaction(ElementType damageElem, ElementType attachedElem);
    int applyReactionEffect(ReactionType react, ElementType damageElem, bool& isFrozen, bool& isElectro);
    void dealElementDamage(ElementType damageElement, int baseDamage);

    // 伤害播报函数
    void showDamageBroadcast(bool isEnemyAttack, int damage);

    // ==================== 技能系统核心函数 ====================
    int getRoleQMaxEnergy(RoleID id);                            // 获取角色Q技能所需充能上限
    bool checkDiceForCurrentSkill();                            // 检查骰子是否满足当前技能
    void castSkill_A();                                          // 释放普攻
    void castSkill_E();                                          // 释放E技能
    void castSkill_Q();                                          // 释放Q技能
    void consumeDiceForSkill();                                  // 扣除选中的骰子

    // ==================== 回合显示函数 ====================
    void updateRoundDisplay();

    // ==================== 【新增】放置区（召唤物 + 持续效果） ====================
    void initSummonArea();
    void addSummon(SummonType type, int lastTurns = 2);
    void addStatusEffect(StatusEffectType type, int lastTurns = 2);
    void refreshSummonDisplay();
    void updatePlacedEntriesOnTurnEnd();
    void clearAllPlacedEntries();

    // ==================== 【新增】5张功能牌实现函数 ====================
    void playCard_运筹帷幄();
    void playCard_西风大教堂();
    void playCard_望舒客栈();
    void playCard_交给我吧();
    void playCard_换班时间();

    // ==================== 【新增】伤害记录 ====================
    void addDamageLog(const QString& text);

    // 胜负判定相关（修复编译错误新增）
    void checkActiveRoleDead();      // 检查出战角色是否阵亡
    void checkEnemyDead();           // 检查敌人是否阵亡
    void checkAllRoleDead();         // 检测我方是否全灭
    void switchToHighestHpBackRole();// 自动切血量最高后台角色
    void showGameResult(bool isWin); // 显示胜利/失败界面
    void hideGameResult();           // 隐藏结算界面

private:
    Ui::MainWindow *ui;

    QVector<QLabel*> handLabels;
    QVector<bool> selectedFlags;
    QVector<QLabel*> roleCards;
    QVector<QLabel*> diceLabels;
    QVector<bool> diceSelected;

    QLabel* tooltipLabel;
    QLabel* tipLabel;
    QLabel* reRollTipLabel;
    QPushButton* mulliganBtn;
    QPushButton* rollDiceBtn;
    QPushButton* reRollBtn;
    QPushButton* playCardBtn;

    bool roleSelectDone;
    ElementType currentRoleElement;
    QVector<RoleID> myTeam;
    QSet<ElementType> m_teamElements;
    GameLogic* gameLogic;
    int currentHoverIndex;

    int selectedCardIndex;
    QVector<int> selectedDiceIndices;
    CardCostType currentCostType;

    // 角色状态
    QWidget* roleStatusPanel;
    QVector<QLabel*> roleNameLabels;
    QVector<QLabel*> roleHpLabels;
    QVector<QLabel*> roleAtkLabels;
    QVector<QLabel*> roleElementLabels;
    QVector<QLabel*> roleWeaponLabels;
    QVector<QLabel*> roleEnergyLabels;

    QVector<int> roleHp;
    QVector<int> roleAtk;
    QVector<int> roleEnergy;
    int currentActiveRoleIndex;

    // 敌人
    QLabel* enemyLabel;
    int enemyHp;
    int enemyAtk;
    int enemyEnergy;
    QLabel* enemyHpLabel;
    QLabel* enemyAtkLabel;
    QLabel* enemyEnergyLabel;

    // 敌人元素反应状态
    ElementType enemyAttachedElement;
    bool enemyFrozen;
    bool enemyElectro;

    // 角色切换相关
    QPushButton *switchRoleBtn;
    int selectedSwitchRoleIndex;
    bool isSwitchingRole;

    // 回合系统
    QPushButton *endTurnBtn;
    QLabel *turnTipLabel;
    bool myTurnEnded;
    bool enemyTurnEnded;
    int enemyActionTimes;

    // 操作锁定 & 回合状态
    bool isPlayerActionPhase;

    // 伤害播报标签
    QLabel *damageBroadcastLabel;

    int enemyRemainingActions = 3;

    // ==================== 技能系统 UI 按钮 ====================
    QPushButton *btn_A;
    QPushButton *btn_E;
    QPushButton *btn_Q;
    QPushButton *btn_castSkill;

    // ==================== 技能系统状态 ====================
    SkillType selectedSkill;
    bool skillSelected;

    QLabel *m_roundLabel;
    int m_currentRound;

    // ==================== 【新增】左下角放置区 UI ====================
    QWidget *summonAreaWidget;
    QLabel *summonAreaTitle;
    QVector<QLabel*> summonItemLabels;
    QVector<PlacedEntry> placedEntries;

    // ==================== 【新增】卡牌效果开关 ====================
    bool m_nextSwitchFreeAction;  // 交给我吧
    bool m_nextSwitchFreeDice;    // 换班时间

    // ==================== 【新增】伤害记录栏（和召唤物区域同款样式） ====================
    QWidget *m_damageLogWidget;
    QLabel *m_damageLogTitle;
    QVector<QLabel*> m_damageLogLabels;
    QStringList m_damageLog;

    // 新增：胜利失败弹窗
    QLabel *m_gameResultLabel;    // 游戏胜利/失败大字
    QPushButton *m_restartBtn;    // 重新开始按钮
};

#endif // MAINWINDOW_H