#pragma once

#include "Game.h"
#include <iostream>

namespace Game {
    struct CompTransform : Component {
    private:
        Vec2 mCoord;
        float mScale = 1;
        float mAngle = 0;

    public:
        void Coord(const Vec2 & coord) { mCoord = coord; }
        void Scale(float scale) { mScale = scale; }
        void Angle(float angle) { mAngle = angle; }
        const Vec2 & Coord() { return mCoord; }
        float Scale() { return mScale; }
        float Angle() { return mAngle; }

        virtual void OnUpdate(float dt) override { }
        virtual void OnEnter() override { }
        virtual void OnLeave() override { }
        virtual const type_info & GetType() override
        {
            return typeid(CompTransform);
        }
    };

    struct CompSprite : Component {
    private:
        std::vector<Simple2D::Image *> mFrames;

    public:
        uint mCurr;
        Vec2 mAnchor;
        bool mIsLoop;
        float mInterval;
        float mLoopTime;

        void AddFrame(Simple2D::Image * image)
        {
            mFrames.push_back(image);
        }

        virtual void OnUpdate(float dt)
        {
            mLoopTime += dt;
            mCurr = (uint)(mLoopTime / mInterval);
            mCurr = mIsLoop ? (uint)(mCurr  % mFrames.size())
                            : std::min(mCurr, mFrames.size() - 1);

            auto w = 0, h = 0;
            auto frame = mFrames.at(mCurr);
            Simple2D::GetImageSize(frame, &w, &h);
            Simple2D::DrawImage(frame,
                                mOwner->mTrans->Coord().x - w * mAnchor.x,
                                mOwner->mTrans->Coord().y - h * mAnchor.y,
                                mOwner->mTrans->Angle(), mOwner->mTrans->Scale());
        }

        virtual void OnEnter() override { }
        virtual void OnLeave() override { }
        virtual const type_info & GetType() override
        {
            return typeid(CompSprite);
        }
    };

    struct CompText : Component {
    private:
        std::string      mText;
        Simple2D::Font * mFont;

    public:
        std::string & Text()
        {
            return mText;
        }

        Simple2D::Font *& Font()
        {
            return mFont;
        }

        virtual void OnUpdate(float dt) override
        {
            Simple2D::DrawString(mFont, mText,
                                 mOwner->mTrans->Coord().x,
                                 mOwner->mTrans->Coord().y,
                                 mOwner->mTrans->Angle(),
                                 mOwner->mTrans->Scale());
        }

        virtual void OnEnter() override { }
        virtual void OnLeave() override { }
        virtual const type_info & GetType() override
        {
            return typeid(CompText);
        }
    };

    //  ¹öÆÁ
    struct CompScrollScreen : Component {
    private:
        Simple2D::Image * mImage;
        Vec2 mScreenSize;
        Vec2 mOriginSize;
        Vec2 mCoord;
        Vec2 mSpeed;
        Vec2  mZero;
        float mSign;
        float mBase;

    public:
        void SetImage(Simple2D::Image * image)
        {
            mImage = image;
            auto w = 0, h = 0;
            Simple2D::GetImageSize(image, &w, &h);
            mOriginSize.x = (float)w;
            mOriginSize.y = (float)h;
        }

        void SetScreenSize(const Vec2 & vec2)
        {
            mScreenSize = vec2;
            mCoord.x = 0;
            mCoord.y = 0;
            mOwner->mTrans->Coord(vec2 * 0.5f);
        }

        void SetSpeed(const Vec2 & vec2)
        {
            mSpeed = vec2;
            mSign = mSpeed.x != 0 ? std::signbit(mSpeed.x) ? -1.0f : 1.0f
                                  : std::signbit(mSpeed.y) ? -1.0f : 1.0f;
            mZero.x = (mSpeed.x != 0.0f ? 1.0f : 0.0f);
            mZero.y = (mSpeed.y != 0.0f ? 1.0f : 0.0f);
            mBase = mSign > 0.0f ? 0.0f : 1.0f;
        }

        virtual void OnUpdate(float dt) override
        {
            mCoord = mCoord + mSpeed * dt;

            auto coord = mCoord * mZero;
            DrawImage(coord + mSign * Vec2(mScreenSize.x * 0.00f, mScreenSize.y * 0.00f));
            DrawImage(coord + mSign * Vec2(mScreenSize.x * 0.25f, mScreenSize.y * 0.25f));
        }

        void DrawImage(const Vec2 & coord)
        {
            auto u = coord.x / (mScreenSize.x * 0.5f); u -= std::floor(std::abs(u)) * mSign;
            auto v = coord.y / (mScreenSize.y * 0.5f); v -= std::floor(std::abs(u)) * mSign;
            auto fx = Math::Lerp(-mOriginSize.x, mOriginSize.x, u + mBase); fx *= mZero.x;
            auto fy = Math::Lerp(-mOriginSize.y, mOriginSize.y, v + mBase); fy *= mZero.y;
            Simple2D::DrawImage(mImage,
                mOwner->mTrans->Coord().x + fx,
                mOwner->mTrans->Coord().y + fy,
                mOwner->mTrans->Angle(),
                mOwner->mTrans->Scale());
        }

        virtual void OnEnter() override { }
        virtual void OnLeave() override { }
        virtual const type_info & GetType() override
        {
            return typeid(CompScrollScreen);
        }
    };
}
