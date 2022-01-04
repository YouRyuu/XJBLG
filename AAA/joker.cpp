//
// Created by youryuu on 2022/1/3.
// 模拟炸金花小程序
//
/*
 * 思路：
 * A到K总共52张牌，按照黑桃梅花红桃方块的顺序依次排列，AAAA22223333...
 * 模拟一次发牌：随机生成一个数字N[0,52),然后取到的牌面值就是N除以4的商+1,花色就是N对4取余
 * 定义一个全局数组used[52],用来记录已经发出去的牌，若某张牌(N)已经发了，则used[N]=1。上面那一步的发牌只选择used[N]=0的。
 * 没有验证这个算法是否公平
 *
 * 类的设计思路:
 * 人依次获取三张牌
 */

#include "joker.h"

using namespace std;

void showCards(int value, Color color)        //将牌在命令行中表示出来
{
    string valShow;
    string colShow;
    switch (value) {
        case 14: valShow="A";break;
        case 11: valShow="J";break;
        case 12: valShow="Q";break;
        case 13: valShow="K";break;
        default: valShow=to_string(value);
    }

    switch (color) {
        case Diamond: colShow="♦️";break;
        case Heart: colShow="♥️";break;
        case Club: colShow="♣️";break;
        case Spade: colShow="♠️";break;
        default:colShow="error";
    }

    cout<<colShow<<valShow;
}

/*
 * 纸牌类，一张纸牌含有以下属性:
 * 牌面值，花色
 */
class Card
{
public:
    Card():value(0),color(Diamond),rank(0)
    {    }

    void setValue(int _value)
    {
        value=_value;
    }

    void setColor(Color _color)
    {
        color=_color;
    }

    void setRank(int _rank)
    {
        rank=_rank;
    }

    int getValue() const
    {
        return value;
    }

    int getRank () const
    {
        return rank;
    }

    Color getColor() const
    {
        return color;
    }

    bool operator<(const Card& ca)  const    //定义单张牌的大小比较，先看牌面值，如果牌面值相等就比较花色
    {
        if(rank!=ca.getRank())
        {
            return rank<ca.getRank();
        }
        else
        {
            return color<ca.getColor();
        }
    }

private:
    int value;      //牌面值
    Color color;    //花色
    int rank;       //牌的大小
};

class Player:public std::enable_shared_from_this<Player>        //赌徒
{
    /*
     * 玩家类，代表某一个玩家，拥有昵称、一副手牌：用一个vector表示拥有的三张牌
     * 获取牌、弃牌、洗牌、比牌(定义两个玩家之间的牌面值比较函数)
     */
public:
    explicit Player(string _nickname):rank(0), nickname(std::move(_nickname)), state(pwait)
    {
        cards.reserve(3);
    }

    //内置操作

    bool operator<(const Player& p2)  const   //两个玩家之间的牌面值比较函数
    {
        /*
         * 先比较类型、再比较牌面值大小
         * 类型：散牌<对子<顺子<清一色<顺金<豹子
         * 牌面值：不同的牌面值，比较最大的值；相同的牌面值，比较花色：黑桃>梅花>红桃>方块
         * 此处比较算法参考：https://zhuanlan.zhihu.com/p/383803321，但是这个算法有缺陷：
         * TODO:没有考虑牌面值相同但是花色不同的比较大小问题，这个可以参考：https://blog.csdn.net/zyzhehe/article/details/53858436?locationNum=7&fps=1
         * 炸弹 = 1249965 + 权重分 （ max: AAA = 1252918, min: 222 = 1249979），
         * 顺金 = 626460 + 顺子 （ max: 同花QKA = 1249964, min: 同花A23 = 670785），
         * 同花 = 623505 + 权重分 ( max: 同花JKA = 626459, min: 同花245 = 623519)，
         * 常规顺子 = 44325 * 权值（ max：14，min：2） + 权重分  ( max: QKA = 623504, min: 234 = 88662)，
         * 特殊顺子 = 44325  ( 即：A23 = 固定分：44325)，
         * 对子 = 2955 * 权值（ max：14，min：2） + 权重分  （ max: AAK = 44324, min: 223 = 5924），
         * 单张 = 权重分（ max：AAA = 2954分，min：222 = 14分）
         */
        return rank<p2.getRank();
    }

    void setState(PlayerState _state)
    {
        //设置玩家的状态
        state=_state;
    }

    int getRank() const
    {
        return rank;
    }

    string getName() const
    {
        return nickname;
    }

    PlayerState getState() const
    {
        return state;
    }

    //被动操作；一定会执行的操作

    void getCard(Card _card)     //获取一张牌
    {
        cards.push_back(_card);
        state=pblind;       //拿到牌后默认暂时未看牌
    }

    void sortPlayerCards()      //对玩家的手牌进行大小排序,先看牌面值，如果牌面值相等就比较花色，从小到大排序
    {
        sort(cards.begin(), cards.end(), [](const Card& c1, const Card& c2) {return c1<c2;});
    }

    int figureColor()       //计算花色对应的rank
    {
        if(cards[0].getColor()==cards[1].getColor() && cards[1].getColor()==cards[2].getColor())        //同花
            return 623505;
        else
            return 0;
    }

    int figureValue()        //计算牌面值对应的rank值
    {
        int _baseRank = pow(cards[0].getRank(),1)+pow(cards[1].getRank(), 2)+pow(cards[2].getRank(), 3);    //牌面值基础分,最大值2954
        if(cards[0].getRank()==cards[1].getRank() && cards[1].getRank()==cards[2].getRank())        //豹子
            return _baseRank+1249965;
        else if(cards[0].getRank()+1==cards[1].getRank() && cards[1].getRank()+1==cards[2].getRank())     //常规顺子，不包括A23
            return 44325*cards[2].getRank()+_baseRank;
        else if(cards[0].getRank()+1==cards[1].getRank() && cards[2].getRank()==14)     //A23顺子
            return 44325;
        else if(cards[0].getRank()==cards[1].getRank() || cards[1].getRank()==cards[2].getRank())       //对子
            return 2955*cards[1].getRank() + _baseRank;
        else        //散牌
            return _baseRank;
    }

    void figureRank()        //计算牌面rank（花色+牌面值)
    {
        rank = figureValue() + figureColor();
    }

    //主动操作

    void setDown(Dealer& dealer);       //找到牌桌坐下

    void leave(Dealer& dealer);         //离开牌桌，这个函数只能在玩家处于pwait或者palready状态时候才能调用

    void discard()     //弃牌: 1.玩家自己主动弃牌；2.在对局结束后，由洗牌函数调用
    {
        cards.clear();
        state=palready;        //状态变为弃牌。
    }

    void showPlayerCards()      //输出玩家的手牌
    {
        state=pknow;        //状态变为已看牌
        cout<<nickname<<"的手牌：";
        for(vector<Card>::iterator it=cards.begin();it!=cards.end();it++)
        {
            showCards(it->getRank(), it->getColor());
        }
        cout<<"\trank:"<<rank<<endl;
    }

    CompareResult compareWithOther(Player& op, Dealer& dealer);     //和另外一个人比牌

private:
    int rank;   //手牌的权重值rank，用来比较牌面大小
    string nickname;  //玩家昵称
    vector<Card> cards;     //拥有的三张牌,按照升序排序：5QA
    PlayerState state;      //所处状态
};


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



void Player::setDown(Dealer &dealer)
{
    dealer.addPlayer(shared_from_this());
    //dealer.addPlayer( static_cast<shared_ptr<Player>>(this));
}

void Player::leave(Dealer &dealer)
{
    dealer.deletePlayer(shared_from_this());
}

CompareResult Player::compareWithOther(Player &op, Dealer &dealer)
{
    return dealer.compareTwoPlayers(this->shared_from_this(), op.shared_from_this());
}

void playerMenu(shared_ptr<Player>player, Dealer& dealer)
{
    /*
     * 玩家的操作菜单
     * 要根据玩家现在的状态来选择操作：
     * pwait:找桌子坐下setDown();
     * palready:离开此牌桌leave();
     * pblind:看牌showPlayerCard()、跟人比牌compareWithOther()、下注
     * pknow:跟人比牌compareWithOther()、弃牌discard()、下注
     */
    cout<<"请选择操作"<<endl;
    PlayerState state = player->getState();
    switch(state)
    {
        case pwait:cout<<"1.就座"<<endl;break;
        case palready:cout<<"1.离开"<<endl;break;
        case pblind:cout<<"1.看牌\t2.比牌\t3.下注";break;
        case pknow:cout<<"1.比牌\t2.下注\t3.弃牌";break;
        default:cout<<"欢迎来到CPNP club！"<<endl;break;
    }
    int no;
    cin>>no;
    switch(no)
    {
        case 1:
        {
            if(state==pwait)
            {
                player->setDown(dealer);
            }
            else if(state==palready)
            {
                player->leave(dealer);
            }
            else if(state==pblind)
            {
                player->showPlayerCards();
            }
            else
            {
                cout<<"选择了比牌"<<endl;
            }
            break;
        }
        case 2:
        {
            if(state==pblind)
            {
                cout<<"选择了比牌"<<endl;
            }
            else if(state==pknow)
            {
                cout<<"选择了下注"<<endl;
            }
            else
            {
                cout<<"无效操作"<<endl;
            }
            break;
        }
        case 3:
        {
            if(state==pblind)
            {
                cout<<"选择了下注"<<endl;
            }
            else if(state==pknow)
            {
                player->discard();
            }
            else
            {
                cout<<"无效操作"<<endl;
            }
            break;
        }
        default:break;
    }
}

void test()
{
    Dealer dealer;
    shared_ptr<Player> p1(new Player("卢本伟"));
    shared_ptr<Player> p2(new Player("马飞飞"));
    shared_ptr<Player> p3(new Player("刘某人"));
    shared_ptr<Player> p4(new Player("内卷王"));
    shared_ptr<Player> p5(new Player("汉堡包"));
    playerMenu(p1, dealer);
    playerMenu(p2, dealer);
    playerMenu(p3, dealer);
}