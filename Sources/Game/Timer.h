#pragma once

#include <queue>
#include <functional>

class Timer {
public:
    using Handler = std::function<void()>;

    struct Item {
        int     mID;
        float   mTime;
        Handler mFunc;

        Item()
        { }

        Item(int id, float time, const Handler & func) : mID(id), mTime(time), mFunc(func)
        { }

        bool operator < (const Item & v)
        {
            return mTime < v.mTime;
        }

        bool operator > (const Item & v)
        {
            return mTime > v.mTime;
        }
    };

    Timer() : mID(0)
    {
    }

    ~Timer()
    {
    }

    int Reg(float time, const Handler & func)
    {
        Item item(++mID, time, func);
        mItems.push_back(item);
        std::push_heap(mItems.begin(), mItems.end());
        return item.mID;
    }

    void Del(int id)
    {
        auto fn = [id] (const auto & v) { return v.mID == id; };
        auto it = std::remove_if(mItems.begin(), mItems.end(), fn);
        if (it != mItems.end())
        {
            mItems.erase(it); std::make_heap(mItems.begin(), mItems.end());
        }
    }

    void Call(float time)
    {
        for (; !mItems.empty() && mItems.front().mTime >= time;)
        {
            const auto&top = mItems.front();top.mFunc();
            std::pop_heap(mItems.begin(), mItems.end());
            mItems.pop_back();
        }
    }

    int mID;
    std::vector<Item> mItems;
};
