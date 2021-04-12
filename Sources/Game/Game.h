#pragma once

#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <typeinfo>
#include "Math.h"
#include "Timer.h"
#include "Simple2D.h"

using uint = std::uint32_t;

namespace Game {
    static int mWindowW = 800;
    static int mWindowH = 600;

    enum class InputEnum {
        kDirL = 0x1,
        kDirR = 0x2,
        kDirU = 0x4,
        kDirD = 0x8,
        kFire = 0x10,
    };

    enum class PlayState {
        kMenu,      //  ��Ϸ�˵�
        kInit,      //  ��ʼ��
        kIdle,      //  ����״̬
        kBattle0,   //  Boss�������
        kBattle1,   //  Boss�������
        kBattle2,   //  Boss�������
        kSuccess,   //  ʤ��
        kFailed,    //  ʧ��
    };

    enum class CollisionTag {
        kBullet = 0x1,
        kPlayer = 0x2,
        kEnemy  = 0x4,
    };

    struct Actor;
    struct Component;
    struct CompTransform;

    struct Component {
    public:
        Actor * mOwner;
        virtual ~Component() {}
        virtual void OnEnter() = 0;
        virtual void OnLeave() = 0;
        virtual void OnUpdate(float dt) = 0;
        virtual const type_info & GetType() = 0;

        friend struct Actor;
    };

    struct Actor {
    public:
        uint mID;
        std::string mTag;
        CompTransform * mTrans;
        std::vector<Component *> mComps;

        ~Actor()
        {
            for (auto comp : mComps)
            {
                delete comp;
            }
        }

        template <typename T>
        T * AddComponent()
        {
            auto comp = mComps.emplace_back(new T());
            if (typeid(T) == typeid(CompTransform))
            {
                mTrans = (CompTransform *)comp;
            }
            comp->mOwner = this;
            return (T *)comp;
        }

        template <typename T>
        T * GetComponent()
        {
            const auto & type = typeid(T);
            auto fn = [&] (const auto & v) { return v->GetType() == type; };
            auto it = std::find_if(mComps.begin(), mComps.end(), fn);
            return (T *)(it != mComps.end() ? *it : nullptr);
        }
    };

    struct GamePlay {
        Vec2        mRange;         //  ��̨��Χ
        PlayState   mState;         //  Play״̬
        std::map<uint, Actor *> mActors;
        std::vector<Actor *> mDeletes;
        std::vector<Actor *> mAppends;
    };

    struct Contex {
        uint      mGID;     //  ����ΨһID
        uint      mInput;   //  ��ǰ����
        Timer     mTimer;   //  ��ʱ��
        float     mLastTime;    //  �����Ӧʱ��
        float     mDiffTime;    //  ��ǰ��Ӧʱ��
        Simple2D::Font * mFont24;   //  ����24
        Simple2D::Font * mFont36;   //  ����26
        Simple2D::Font * mFont72;   //  ����72
        std::map<std::string, Simple2D::Image *> mImages;

        GamePlay mPlay;
    };

    Contex * Ctx();
    Actor * AppendActor();
    void    DeleteActor(Actor * actor);
    Actor * FindActor(uint id);
    Actor * FindActor(const std::string & tag);

    void UpdateInput();
    void UpdateActor();
    void GameInit();
    void GameStep();
    void GameStart();
}
