//
// Created by youryuu on 2022/1/3.
//

#ifndef AAA_JOKER_H
#define AAA_JOKER_H

#endif //AAA_JOKER_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

class Card;
class Player;
class Dealer;

enum Color {Diamond, Heart, Club,Spade};   //方块、红桃、梅花、黑桃，这样做是为了计算牌大小方便，同等面值黑桃最大
enum PlayerState {pwait, pblind, pknow, pfold};        //玩家所处状态：等待发牌wait、未看牌、已经看牌、弃牌
void test();

typedef bool (*compareTP)(std::shared_ptr<Player>& p1, std::shared_ptr<Player>& p2, void* obj);        //两个玩家比较大小的回调函数