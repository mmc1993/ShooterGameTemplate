#pragma once

#include "Game.h"
#include "Component.h"

namespace Play {
    //  背景
    struct Background : public Game::Component {
    private:
        Game::CompScrollScreen * mScroll;

    public:
        virtual void OnEnter() override
        {
            mOwner->mTrans->Coord(Vec2(Game::mWindowW * 0.5f,
                                       Game::mWindowH * 0.5f));

            auto scroll = mOwner->AddComponent<Game::CompScrollScreen>();
            scroll->SetImage(Game::Ctx()->mImages.at("StarLayer"));
            scroll->SetScreenSize(Vec2((float)Game::mWindowW,
                                       (float)Game::mWindowH));
            scroll->SetSpeed(Vec2(0, 0));
            mScroll = scroll;
        }

        virtual void OnLeave() override
        { }

        virtual void OnUpdate(float dt) override
        {
            switch (Game::Ctx()->mPlay.mState)
            {
            case Game::PlayState::kSuccess:
            case Game::PlayState::kFailed:
            case Game::PlayState::kMenu: mScroll->SetSpeed(Vec2(0, 0)); break;

            case Game::PlayState::kInit:
            case Game::PlayState::kIdle: mScroll->SetSpeed(Vec2(-10, 0)); break;

            case Game::PlayState::kBattle0:
            case Game::PlayState::kBattle1:
            case Game::PlayState::kBattle2: mScroll->SetSpeed(Vec2(-50, 0)); break;
            }
        }

        virtual const type_info & GetType() override
        {
            return typeid(Background);
        }
    };

    //  碰撞
    struct Collision : public Game::Component {
    private:
        void OnHit(Component * comp)
        {
            if (comp->GetType() != typeid(Collision) || comp->mOwner == mOwner || mHitFn == nullptr)
            {
                return;
            }

            auto other = (Collision *)comp;
            if ((other->mSelf & mMask) == 0)
            {
                auto a = Cir(       mOwner->mTrans->Coord(),        mRadius);
                auto b = Cir(other->mOwner->mTrans->Coord(), other->mRadius);
                if (Math::IsContains(a, b)) { mHitFn(other); }
            }
        }

    public:
        uint mSelf; //  自身
        uint mMask; //  屏蔽
        float mRadius;
        std::function<void(Collision *)> mHitFn;

        virtual void OnEnter() override
        { }

        virtual void OnLeave() override
        { }

        virtual void OnUpdate(float dt) override
        {
            auto & coord = mOwner->mTrans->Coord();
            if (coord.x + mRadius * 10 < 0 ||
                coord.y + mRadius * 10 < 0 ||
                coord.x - mRadius * 10 > Game::Ctx()->mPlay.mRange.x ||
                coord.y - mRadius * 10 > Game::Ctx()->mPlay.mRange.y)
            {
                Game::DeleteActor(mOwner);
            }
            else
            {
                for (auto actor : Game::Ctx()->mPlay.mActors)
                {
                    std::for_each(actor.second->mComps.begin(), actor.second->mComps.end(),
                        std::bind(&Collision::OnHit, this, std::placeholders::_1));
                }
            }
        }

        virtual const type_info & GetType() override
        {
            return typeid(Collision);
        }
    };

    //  战斗中

    //  结束
    struct GameOver : public Game::Component {
        virtual void OnEnter() override
        {
            mOwner->mTrans->Coord(Vec2(Game::mWindowW * 0.5f,
                                       Game::mWindowH * 0.5f));

            if (Game::Ctx()->mPlay.mState == Game::PlayState::kSuccess)
            {
                auto text = mOwner->AddComponent<Game::CompText>();
                text->Font() = Game::Ctx()->mFont72;
                text->Text() = "You Not Loser!!!!!";
            }
            else
            {
                auto text = mOwner->AddComponent<Game::CompText>();
                text->Font() = Game::Ctx()->mFont72;
                text->Text() = "You Is Loser!!!!!!";
            }
        }

        virtual void OnLeave() override
        { }

        virtual void OnUpdate(float dt) override
        { }

        virtual const type_info & GetType() override
        {
            return typeid(GameOver);
        }
    };

    //  爆炸
    struct Boom : public Game::Component {
        float mTime;
        Vec2 mCoord;

        virtual void OnEnter() override
        {
            mOwner->mTrans->Coord(mCoord);

            auto sprite = mOwner->AddComponent<Game::CompSprite>();
            sprite->mCurr = 0;
            sprite->mIsLoop = false;
            sprite->mInterval = 0.1f;
            sprite->AddFrame(Game::Ctx()->mImages.at("Explosion_1"));
            sprite->AddFrame(Game::Ctx()->mImages.at("Explosion_2"));

            mTime = 0.2f;
        }

        virtual void OnLeave() override
        { }

        virtual void OnUpdate(float dt) override
        {
            mTime = std::max(0.0f, mTime - dt);

            auto coord = mOwner->mTrans->Coord();
            coord.x -= dt * 500;
            mOwner->mTrans->Coord(coord);

            if (mTime == 0) { Game::DeleteActor(mOwner); }
        }

        virtual const type_info & GetType() override
        {
            return typeid(GameOver);
        }
    };

    //  子弹
    struct Bullet : public Game::Component {
        bool mIsDie;
        float mRadius;          //  半径
        Vec2 mSpeed;            //  速度
        Vec2 mCoord;            //  初始坐标
        uint mSelf;             //  自身碰撞Tag
        uint mMask;             //  屏蔽碰撞Tag
        std::string mImage;     //  自身贴图

        virtual void OnEnter() override
        {
            mIsDie = false;

            mOwner->mTrans->Coord(mCoord);

            auto collision = mOwner->AddComponent<Collision>();
            collision->mRadius = mRadius;
            collision->mSelf = mSelf;
            collision->mMask = mMask;
            collision->mHitFn = std::bind(&Bullet::OnHit, this, std::placeholders::_1);

            auto sprite = mOwner->AddComponent<Game::CompSprite>();
            sprite->mIsLoop = false; sprite->mInterval = 1.0f;
            sprite->mAnchor = Vec2(0.0f, 0.0f); sprite->mCurr = 0;
            sprite->AddFrame(Game::Ctx()->mImages.at(mImage));
        }

        void OnHit(Collision * comp)
        {
            if (!mIsDie)
            {
                auto actor = Game::AppendActor();
                auto boom = actor->AddComponent<Boom>();
                boom->mCoord = mOwner->mTrans->Coord();
                mIsDie = true;
            }
            
        }

        virtual void OnLeave() override
        { }

        virtual void OnUpdate(float dt) override
        {
            if (!mIsDie)
            {
                auto coord = mOwner->mTrans->Coord();
                mOwner->mTrans->Coord(coord + mSpeed * dt);
                mOwner->mTrans->Angle(Math::ToAngle(mSpeed));
            }
            else
            {
                Game::DeleteActor(mOwner);
            }
        }

        virtual const type_info & GetType() override
        {
            return typeid(Bullet);
        }
    };

    struct Boss : public Game::Component {
        struct Item {
            Vec2 mSpeed;
            Vec2 mCoord;
            Game::Actor * mActor;
        };
        std::vector<Item> mItems;
        Beizer mPaths[3];
        size_t mIndex;
        float mMoveTime;
        float mFireTime;
        size_t mFireIdx;
        int mHp;

        bool UpdateCoord(float dt)
        {
            auto & first = mItems.at(0);
            auto & coord = first.mCoord;
            auto preCoord= first.mCoord;

            mMoveTime = std::min(1.0f, mMoveTime + dt *0.1f);
            coord = mPaths[mIndex].Calc(mMoveTime);
            for (auto i = 0; i != mItems.size(); ++i)
            {
                auto & item = mItems.at(i);
                if (i != 0)
                {
                    item.mCoord.x += item.mSpeed.x;
                    item.mCoord.y += item.mSpeed.y;
                    auto & a = mItems.at(i - 1);
                    auto & b = mItems.at(i    );
                    b.mCoord = Math::LimitLength(a.mCoord, b.mCoord, 50);

                    auto angle = Math::ToAngle(item.mCoord - preCoord);
                    item.mActor->mTrans->Angle(angle);
                }
                item.mActor->mTrans->Coord(item.mCoord);

                item.mSpeed.x *= 0.8f;
                item.mSpeed.y *= 0.8f;
            }
            return mMoveTime == 1.0f;
        }

        void AppendItem()
        {
            auto first  = mItems.empty();
            auto & item = mItems.emplace_back();

            auto actor = Game::AppendActor();

            actor->mTrans->Coord(Vec2((float)Game::mWindowW, (float)Game::mWindowH));

            auto sprite = actor->AddComponent<Game::CompSprite>();
            sprite->mInterval = 1; sprite->mIsLoop = false; sprite->mCurr = 0;
            sprite->AddFrame(Game::Ctx()->mImages.at(first ? "Enemy_3" : "Enemy_1"));

            auto collision = actor->AddComponent<Collision>();
            collision->mHitFn = std::bind(&Boss::OnHit, this,
                    std::placeholders::_1, mItems.size() - 1);
            collision->mSelf = (int)Game::CollisionTag::kEnemy;
            collision->mMask = (int)Game::CollisionTag::kEnemy;
            collision->mRadius = first ? 50.0f : 20.0f;

            item.mActor = actor;
        }

        virtual void OnEnter() override
        {
            mPaths[0].InitBeizer({
                { Game::mWindowW * 0.7f, Game::mWindowH * 0.5f },
                { Game::mWindowW * 0.1f, Game::mWindowH * 0.9f },
                { Game::mWindowW * 0.1f, Game::mWindowH * 0.1f },
                { Game::mWindowW * 0.9f, Game::mWindowH * 0.1f },
                { Game::mWindowW * 0.9f, Game::mWindowH * 0.9f },
                { Game::mWindowW * 0.7f, Game::mWindowH * 0.5f },
            });

            mPaths[1].InitBeizer({
                { Game::mWindowW * 0.7f, Game::mWindowH * 0.5f },
                { Game::mWindowW * 0.1f, Game::mWindowH * 0.1f },
                { Game::mWindowW * 0.1f, Game::mWindowH * 0.9f },
                { Game::mWindowW * 0.5f, Game::mWindowH * 0.1f },
                { Game::mWindowW * 0.9f, Game::mWindowH * 0.9f },
                { Game::mWindowW * 0.9f, Game::mWindowH * 0.1f },
                { Game::mWindowW * 0.7f, Game::mWindowH * 0.5f },
            });

            mPaths[2].InitBeizer({
                { Game::mWindowW * 0.7f, Game::mWindowH * 0.5f },
                { Game::mWindowW * 0.1f, Game::mWindowH * 0.1f },
                { Game::mWindowW * 0.1f, Game::mWindowH * 0.9f },
                { Game::mWindowW * 0.9f, Game::mWindowH * 0.1f },
                { Game::mWindowW * 0.9f, Game::mWindowH * 0.9f },
                { Game::mWindowW * 0.6f, Game::mWindowH * 0.5f },
                { Game::mWindowW * 0.7f, Game::mWindowH * 0.5f },
            });

            mHp = 100;
            mIndex = 1;
            mMoveTime = 0.0f;
            mFireTime = 0.0f;
        }

        void OnHit(Collision * comp, size_t index)
        {
            auto item = &mItems.at(index);
            auto & fCoord = comp->mOwner->mTrans->Coord();
            auto & tCoord = item->mActor->mTrans->Coord();
            item->mSpeed = Math::Normal(tCoord - fCoord);
            item->mSpeed = item->mSpeed * 30 * 0.01f * (float)index;

            if (--mHp == 0)
            {
                Game::Ctx()->mPlay.mState = Game::PlayState::kSuccess;

                auto actor = Game::AppendActor();
                actor->AddComponent<GameOver>();

                for (auto & item : mItems)
                {
                    auto actor  = Game::AppendActor();
                    auto boom   = actor->AddComponent<Boom>();
                    boom->mCoord = item.mActor->mTrans->Coord();

                    Game::DeleteActor(item.mActor);
                }
                Game::DeleteActor(mOwner);
            }
        }

        void Fire()
        {
            mFireIdx = (mFireIdx + 1) % mItems.size();

            auto &item = mItems.at(mFireIdx);
            auto actor = Game::AppendActor();
            auto bullet = actor->AddComponent<Bullet>();
            bullet->mCoord = item.mActor->mTrans->Coord();
            bullet->mImage = "EnemyBullet";
            bullet->mMask = (int)Game::CollisionTag::kEnemy;
            bullet->mSelf = (int)Game::CollisionTag::kEnemy
                            | (int)Game::CollisionTag::kBullet;
            bullet->mRadius = 25;

            bullet->mSpeed.x = Math::Random(-1.0f, 2.0f);
            bullet->mSpeed.y = Math::Random(-1.0f, 2.0f);
            bullet->mSpeed = Math::Normal(bullet->mSpeed) * 500;
        }

        virtual void OnLeave() override
        { }

        virtual void OnUpdate(float dt) override
        {
            if (UpdateCoord(dt))
            {
                mIndex = Math::Random(0, 2);
                mMoveTime = 0;
            }
            auto a = mItems.front().mActor->mTrans->Angle();
            mItems.front().mActor->mTrans->Angle(a + 10.0f);

            mFireTime = std::max(0.0f, mFireTime - dt);
            if (mFireTime == 0) { Fire(); mFireTime = 0.1f; }
        }

        virtual const type_info & GetType() override
        {
            return typeid(Boss);
        }
    };

    //  Hero
    struct Hero : public Game::Component {
    public:
        float   mFireTM;    //  开火时间
        float   mFireCD;    //  开火间隔
        float   mRadius;
        Vec2    mSpeed;
        int     mHp;

        virtual void OnEnter() override
        {
            mFireCD = 0.1f;
            mRadius = 50;
            mSpeed.x = 0;
            mSpeed.y = 0;
            mHp = 100;

            mOwner->mTrans->Coord(Vec2(Game::mWindowW * 0.3f,
                                       Game::mWindowH * 0.5f));

            auto collision = mOwner->AddComponent<Collision>();
            collision->mRadius = mRadius;
            collision->mSelf = (int)Game::CollisionTag::kPlayer;
            collision->mMask = (int)Game::CollisionTag::kPlayer;
            collision->mHitFn  = std::bind(&Hero::OnHit, this, std::placeholders::_1);

            auto sprite = mOwner->AddComponent<Game::CompSprite>();
            sprite->mIsLoop = false; sprite->mInterval = 1.0f;
            sprite->mAnchor = Vec2(0.0f, 0.0f); sprite->mCurr = 0;
            sprite->AddFrame(Game::Ctx()->mImages.at("Player_3"));
        }

        void OnHit(Collision * comp)
        {
            mHp = comp->mSelf == (int)Game::CollisionTag::kEnemy
                                      ? 0: std::max(0, mHp - 1);

            if (mHp == 0)
            {
                Game::Ctx()->mPlay.mState = Game::PlayState::kFailed;

                {
                    auto actor = Game::AppendActor();
                    actor->AddComponent<GameOver>();
                }

                {
                    auto actor = Game::AppendActor();
                    auto boom  = actor->AddComponent<Boom>();
                    boom->mCoord = mOwner->mTrans->Coord();
                }

                Game::DeleteActor(mOwner);
            }
        }

        virtual void OnLeave() override
        { }

        virtual void OnUpdate(float dt) override
        {
            auto keys = (int)Game::InputEnum::kDirU | (int)Game::InputEnum::kDirD
                      | (int)Game::InputEnum::kDirL | (int)Game::InputEnum::kDirR;
            if ((Game::Ctx()->mInput & keys) != 0)
            {
                mSpeed.x = 0; mSpeed.y = 0;
                if      ((Game::Ctx()->mInput & (int)Game::InputEnum::kDirL) != 0) { mSpeed.x = -1; }
                else if ((Game::Ctx()->mInput & (int)Game::InputEnum::kDirR) != 0) { mSpeed.x =  1; }
                if      ((Game::Ctx()->mInput & (int)Game::InputEnum::kDirU) != 0) { mSpeed.y =  1; }
                else if ((Game::Ctx()->mInput & (int)Game::InputEnum::kDirD) != 0) { mSpeed.y = -1; }
                mSpeed = Math::Normal(mSpeed) * 1000;
            }

            auto coord = mOwner->mTrans->Coord();
            coord = Math::LimitCoord(Game::Ctx()->mPlay.mRange,
                            Cir(coord + mSpeed * dt, mRadius));
            mOwner->mTrans->Coord(coord);

            if ((Game::Ctx()->mInput & (int)Game::InputEnum::kFire) != 0)
            {
                if (Simple2D::GetGameTime() - mFireTM > mFireCD)
                {
                    auto actor = Game::AppendActor();

                    auto comp = actor->AddComponent<Bullet>();
                    comp->mCoord = coord;
                    comp->mImage = "PlayerBullet";
                    comp->mSelf = (int)Game::CollisionTag::kPlayer
                                | (int)Game::CollisionTag::kBullet;
                    comp->mMask = (int)Game::CollisionTag::kPlayer;
                    comp->mSpeed = Vec2(2000, 0);
                    comp->mRadius = 30;

                    mFireTM = Simple2D::GetGameTime();
                }
            }

            mSpeed = mSpeed * 0.5f;
        }

        virtual const type_info & GetType() override
        {
            return typeid(Hero);
        }
    };


    //  游戏菜单
    struct Menu : public Game::Component {
    public:
        virtual void OnEnter() override
        {
            mOwner->mTrans->Coord(Vec2(Game::mWindowW * 0.5f,
                                       Game::mWindowH * 0.5f));

            auto text = mOwner->AddComponent<Game::CompText>();
            text->Font() = Game::Ctx()->mFont72;
            text->Text() = "Input Space Start!";
        }

        virtual void OnLeave() override
        { }

        virtual void OnUpdate(float dt) override
        {
            if ((Game::Ctx()->mInput & (int)Game::InputEnum::kFire) != 0)
            {
                Game::Ctx()->mPlay.mState = Game::PlayState::kInit;
                Game::DeleteActor(mOwner);

                {
                    auto actor = Game::AppendActor();
                    actor->AddComponent<Hero>();
                }

                {
                    auto actor = Game::AppendActor();
                    auto boss = actor->AddComponent<Boss>();
                    for (auto i = 0; i != 20; ++i)
                    {
                        boss->AppendItem();
                    }
                }
            }
        }

        virtual const type_info & GetType() override
        {
            return typeid(Menu);
        }
    };
}
