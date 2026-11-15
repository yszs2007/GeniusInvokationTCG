#include "gamelogic.h"
#include "card.h"
#include <QVector>
#include <QString>
#include <algorithm>
#include <QRandomGenerator>

void GameLogic::initActionCardDeck()
{
    actionDeck.clear();
    playerHand.clear();
    canMulligan = true;

    QVector<QString> cardNames = {
        "白铁大剑", "白缨枪", "换班时间", "交给我吧", "旅行剑",
        "蒙德土豆饼", "魔导绪论", "骑士团图书馆", "甜甜花酿鸡", "望舒客栈",
        "西风大教堂", "星天之兆", "鸦羽弓", "运筹帷幄", "最好的伙伴"
    };

    for (const QString& name : cardNames) {
        actionDeck << new ActionCard(name, 2, NoElement, "行动卡效果");
        actionDeck << new ActionCard(name, 2, NoElement, "行动卡效果");
    }

    // ✅ 绝对安全、不报错、无警告的洗牌（Qt专用）
    for (int i = actionDeck.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        qSwap(actionDeck[i], actionDeck[j]);
    }
}

void GameLogic::drawInitialHand()
{
    for (int i = 0; i < 5 && !actionDeck.isEmpty(); ++i) {
        playerHand << actionDeck.takeFirst();
    }
}

void GameLogic::mulligan(const QVector<int>& indexes)
{
    if (!canMulligan || indexes.isEmpty())
        return;

    QVector<int> sortedIndexes = indexes;
    std::sort(sortedIndexes.begin(), sortedIndexes.end(), std::greater<int>());

    QList<ActionCard*> backToDeck;
    for (int i : sortedIndexes) {
        if (i >= 0 && i < playerHand.size()) {
            backToDeck << playerHand.takeAt(i);
        }
    }

    actionDeck << backToDeck;

    // ✅ 再次洗牌（安全版）
    for (int i = actionDeck.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        qSwap(actionDeck[i], actionDeck[j]);
    }

    for (int i = 0; i < backToDeck.size(); ++i) {
        playerHand << actionDeck.takeFirst();
    }

    canMulligan = false;
}

void GameLogic::drawCard()
{
    if (!actionDeck.isEmpty()) {
        playerHand.append(actionDeck.takeFirst());
    }
}