#pragma once

#include "Game.h"
#include "play.h"
#include "Component.h"
#include <iostream>


namespace Game {
    Contex mCtx;

    void GameInit()
    {
        //  初始化资源
        mCtx.mFont24 = Simple2D::CreateFont("../../Content/Fonts/AGENCYB.TTF", 24);
        mCtx.mFont36 = Simple2D::CreateFont("../../Content/Fonts/AGENCYB.TTF", 36);
        mCtx.mFont72 = Simple2D::CreateFont("../../Content/Fonts/AGENCYB.TTF", 72);

        mCtx.mImages.emplace(std::make_pair("Moon", Simple2D::CreateImage("../../Content/Textures/Moon.png")));
        mCtx.mImages.emplace(std::make_pair("Enemy_1", Simple2D::CreateImage("../../Content/Textures/Enemy_1.png")));
        mCtx.mImages.emplace(std::make_pair("Enemy_2", Simple2D::CreateImage("../../Content/Textures/Enemy_2.png")));
        mCtx.mImages.emplace(std::make_pair("Enemy_3", Simple2D::CreateImage("../../Content/Textures/Enemy_3.png")));
        mCtx.mImages.emplace(std::make_pair("Upgrade", Simple2D::CreateImage("../../Content/Textures/Upgrade.png")));
        mCtx.mImages.emplace(std::make_pair("Player_1", Simple2D::CreateImage("../../Content/Textures/Player_1.png")));
        mCtx.mImages.emplace(std::make_pair("Player_2", Simple2D::CreateImage("../../Content/Textures/Player_2.png")));
        mCtx.mImages.emplace(std::make_pair("Player_3", Simple2D::CreateImage("../../Content/Textures/Player_3.png")));
        mCtx.mImages.emplace(std::make_pair("StarLayer", Simple2D::CreateImage("../../Content/Textures/StarLayer.png")));
        mCtx.mImages.emplace(std::make_pair("EnemyBullet", Simple2D::CreateImage("../../Content/Textures/EnemyBullet.png")));
        mCtx.mImages.emplace(std::make_pair("Explosion_1", Simple2D::CreateImage("../../Content/Textures/Explosion_1.png")));
        mCtx.mImages.emplace(std::make_pair("Explosion_2", Simple2D::CreateImage("../../Content/Textures/Explosion_2.png")));
        mCtx.mImages.emplace(std::make_pair("Meteorite_1", Simple2D::CreateImage("../../Content/Textures/Meteorite_1.png")));
        mCtx.mImages.emplace(std::make_pair("Meteorite_2", Simple2D::CreateImage("../../Content/Textures/Meteorite_2.png")));
        mCtx.mImages.emplace(std::make_pair("Meteorite_3", Simple2D::CreateImage("../../Content/Textures/Meteorite_3.png")));
        mCtx.mImages.emplace(std::make_pair("Meteorite_4", Simple2D::CreateImage("../../Content/Textures/Meteorite_4.png")));
        mCtx.mImages.emplace(std::make_pair("PlayerBullet", Simple2D::CreateImage("../../Content/Textures/PlayerBullet.png")));

        //  初始化全局变量
        mCtx.mGID       = 0;
        mCtx.mInput     = 0;
        mCtx.mLastTime  = Simple2D::GetGameTime();

        mCtx.mPlay.mRange = Vec2((float)mWindowW, 
                                 (float)mWindowH);
        mCtx.mPlay.mState = PlayState::kMenu;

        GameStart();
    }

    void GameStep()
    {
        auto now = Simple2D::GetGameTime();
        mCtx.mDiffTime = now - mCtx.mLastTime;
        mCtx.mLastTime = now;

        UpdateInput();
        UpdateActor();

        mCtx.mTimer.Call(Simple2D::GetGameTime());
    }

    void GameStart()
    {
        {
            auto actor = AppendActor();
            actor->AddComponent<Play::Background>();
        }
        
        {
            auto actor = AppendActor();
            actor->AddComponent<Play::Menu>();
        }
    }

    void UpdateInput()
    {
        static std::pair<Simple2D::KeyCode, InputEnum> sInputs[] = {
            { Simple2D::KEY_A, InputEnum::kDirL },
            { Simple2D::KEY_D, InputEnum::kDirR },
            { Simple2D::KEY_W, InputEnum::kDirU },
            { Simple2D::KEY_S, InputEnum::kDirD },
            { Simple2D::KEY_SPACE, InputEnum::kFire },
        };

        for (const auto & pair : sInputs)
        {
            if (Simple2D::IsKeyPressed(pair.first))
            {
                mCtx.mInput |=  (int)pair.second;
            }
            if (Simple2D::IsKeyReleased(pair.first))
            {
                mCtx.mInput &= ~(int)pair.second;
            }
        }
    }

    void UpdateActor()
    {
        //  新增
        for (auto actor : mCtx.mPlay.mAppends)
        {
            mCtx.mPlay.mActors.emplace(actor->mID, actor);
            std::for_each(actor->mComps.begin(), actor->mComps.end(),
                std::bind(&Component::OnEnter, std::placeholders::_1));
        }
        mCtx.mPlay.mAppends.clear();

        //  删除
        for (auto actor : mCtx.mPlay.mDeletes)
        {
            auto n = mCtx.mPlay.mActors.erase(actor->mID);
            if (n != 0)
            {
                std::for_each(actor->mComps.begin(), actor->mComps.end(),
                    std::bind(&Component::OnLeave, std::placeholders::_1));
                delete actor;
            }
        }
        mCtx.mPlay.mDeletes.clear();

        for (auto actor : mCtx.mPlay.mActors)
        {
            std::for_each(actor.second->mComps.begin(), actor.second->mComps.end(),
                std::bind(&Component::OnUpdate, std::placeholders::_1, mCtx.mDiffTime));
        }
    }

    Contex * Ctx()
    {
        return &mCtx;
    }

    Actor * AppendActor()
    {
        auto actor = new Actor();
        actor->mID = mCtx.mGID++;
        actor->AddComponent<CompTransform>();
        mCtx.mPlay.mAppends.emplace_back(actor);
        return actor;
    }

    void DeleteActor(Actor * actor)
    {
        mCtx.mPlay.mDeletes.emplace_back(actor);
    }

    Actor * FindActor(uint id)
    {
        auto it = mCtx.mPlay.mActors.find(id);
        return it != mCtx.mPlay.mActors.end() ? it->second : nullptr;
    }

    Actor * FindActor(const std::string & tag)
    {
        auto fn = [&tag] (auto & v) { return v.second->mTag == tag; };
        auto it = std::find_if(mCtx.mPlay.mActors.begin(), 
                               mCtx.mPlay.mActors.end(), fn);
        return it != mCtx.mPlay.mActors.end() ? it->second : nullptr;
    }
}
