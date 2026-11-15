#ifndef DICE_H
#define DICE_H

#include <QVector>
#include <QRandomGenerator>

enum DieType {
    AnemoDie, GeoDie, ElectroDie, DendroDie, HydroDie, PyroDie, CryoDie, OmniDie
};

class Dice {
public:
    // 声明默认构造函数
    Dice();

    static QVector<DieType> roll8() {
        QVector<DieType> res;
        for (int i=0;i<8;++i)
            res << (DieType)QRandomGenerator::global()->bounded(8);
        return res;
    }
};

#endif