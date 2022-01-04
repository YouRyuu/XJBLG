//
// Created by youryuu on 2022/1/5.
//

#include "dealer.h"

class Dealer
{
    /*
     * 荷官类：负责准备一副牌、开始一场对局、洗牌、发牌、判断玩家之间牌的大小等
     */
public:
    Dealer():used{0}
    {

    }

    void shuffle()
    {
        //洗牌：将used数组清零，并且每位玩家执行弃牌操作
        for(const auto& player:players)
            player->discard();
        memset(used, 0, sizeof used);
    }

    Card dealACard()        //发一张牌
    {
        Card card;
        srand(time(nullptr));      //生成一个随机数
        while(true)
        {
            int val=rand()%52;      //让这个随机数分布在[0,52)
            if(used[val]==0)        //这张牌没有没有发出去
            {
                int _value=val/4+1;
                card.setValue(_value);//获取牌面值
                if(_value==1)
                    card.setRank(14);
                else
                    card.setRank(_value);
                int form=val%4;       //获取花色
                switch(form)
                {
                    case 3:card.setColor(Spade);break;
                    case 2:card.setColor(Club);break;
                    case 1:card.setColor(Heart);break;
                    case 0:card.setColor(Diamond);break;
                    default:cout<<"error value of color"<<endl;
                }
                used[val]=1;
                return card;
            }
            else        //这张牌已经抽出去了，再抽一次
                continue;
        }
    }

    void dealCards()
    {
        shuffle();      //每次发牌之前先洗牌
        for(const auto& player:players)
        {
            /*
             * 对每一位玩家发三张牌，发完牌后进行排序、计算rank操作
             */
            for(int i=0;i<3;i++)
            {
                player->getCard(dealACard());
            }
            player->sortPlayerCards();
            player->figureRank();
            //player->showPlayerCards();      //TODO:测试：输出玩家手中的牌，实际应该在玩家主动调用看牌操作时或者对后比牌的时候调用
        }
    }

    void addPlayer(shared_ptr<Player> _player)
    {
        //增加玩家,最多17名玩家,某位玩家坐在了这个牌桌
        if(players.size()<=17)
        {
            players.push_back(_player);
            _player->setState(palready);
            //_player->setDown(static_cast<shared_ptr<Dealer>>(this));
        }
        else
            cout<<"玩家数量过多"<<endl;
    }

    bool deletePlayer(shared_ptr<Player> _player)
    {
        //移除掉某个玩家
        for(vector<shared_ptr<Player>>::iterator it=players.begin();it!=players.end();it++)
        {
            if(*it==_player)
            {
                (*it)->setState(pwait);
                players.erase(it);
                return true;
            }
        }
        return false;
    }

    CompareResult compareTwoPlayers(shared_ptr<Player> p1, shared_ptr<Player> p2)
    {
        //两个玩家之间的牌面值比较
        //如果p1比p2大，返回1， 反之返回0
        /*
         * 两个玩家比牌的可能性：
         * 1.两个玩家都看牌了，互相比较  p1.state==p2.state==pknow
         * 2.两个玩家都没有看牌，直接蒙开
         * 看了牌的玩家不可以和没有看牌的玩家比牌
         * 不管什么情况最后的牌都是看了的
         * 这里默认p1是主动发起看牌的一方，p2是被看牌的一方
         */
        //p1->setState(pknow);        //状态变为已看牌
        //和另一位玩家比牌
        if(p1->getState()==pknow && p2->getState()!=pknow)        //自己已经看牌了但是对方没有看牌，拒绝操作
        {
            cout<<p2->getName()<<"未看牌，无法和他比牌"<<endl;
            return error;
        }
        else if((p1->getState()==p2->getState()) && (p2->getState()==pknow)||(p2->getState()==pblind))
        {
            //两人均看牌了或者两者均未看牌
            p1->setState(pknow);        //现在均看牌了
            p2->setState(pknow);
            if(*p1<*p2)     //谁输了谁弃牌
            {
                p1->discard();
                return fault;
            }
            else
            {
                p2->discard();
                return win;
            }
        }
        else
            return error;
    }

    void showAllPlayers()
    {
        //输出当前牌桌的所有玩家
        for(auto & player : players)
        {
            cout<<player->getName()<<endl;
        }
    }

    void showEffectivePlayers()
    {
        //输出仍持有手牌的玩家信息
        for(auto &player:players)
        {
            if(player->getState()!=pwait && player->getState()!=palready)
                player->showPlayerCards();
        }
    }

    void compareExistPlayers()
    {
        //比较所有有效玩家的牌，并选择出胜利的那一位。
        sort(players.begin(), players.end(), [](const shared_ptr<Player>& p1, const shared_ptr<Player>& p2){return *p1<*p2;});
        cout<<(*(players.end()-1))->getName()<<endl;
    }

private:
    vector<shared_ptr<Player>> players;     //当前牌桌的所有玩家
    //vector<shared_ptr<Player>> activePlayers;       //当前手牌在手的玩家
    int used[52];       //一个荷官有一副牌,记录牌是否已经发出去了，0代表没发，1代表已经发出去了
};