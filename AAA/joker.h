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
enum PlayerState {pwait, palready, pblind, pknow};        //玩家所处状态：未就座、已就座未参与场上博弈、处于博弈状态未看牌、处于博弈状态已经看牌。弃牌可以看做已就座未参与场上博弈
enum CompareResult {win, fault, error};     //比牌的结果：胜利、失败、错误操作（看牌的和没看牌的比，不允许）
void test();

typedef bool (*compareTP)(std::shared_ptr<Player>& p1, std::shared_ptr<Player>& p2, void* obj);        //两个玩家比较大小的回调函数