#pragma once

#include <cmath>
#include <random>
#include <iostream>

struct Vec2
{
	float x;
	float y;

	Vec2(float _x, float _y) : x(_x), y(_y)
	{ }

	Vec2(float _x) : x(_x), y(_x)
	{ }

	Vec2() : x(0), y(0)
	{ }
};

struct Cir {
	Vec2  mO;
	float mR;

	Cir(const Vec2 & o, float r) : mO(o), mR(r)
	{ }

	Cir(const Vec2 & o) : mO(o), mR(0)
	{ }

	Cir() : mR(0)
	{ }
};

static Vec2 operator + (const Vec2 & a, const Vec2 & b)
{
	return Vec2(a.x + b.x, a.y + b.y);
}

static Vec2 operator - (const Vec2 & a, const Vec2 & b)
{
	return Vec2(a.x - b.x, a.y - b.y);
}

static Vec2 operator * (const Vec2 & a, const Vec2 & b)
{
	return Vec2(a.x * b.x, a.y * b.y);
}

static Vec2 operator * (const Vec2 & a, float b)
{
	return Vec2(b * a.x, b * a.y);
}

static Vec2 operator * (float a, const Vec2 & b)
{
	return Vec2(a * b.x, a * b.y);
}

static Vec2 operator / (const Vec2 & a, const Vec2 & b)
{
	return Vec2(a.x / b.x, a.y / b.y);
}

static Vec2 operator / (const Vec2 & a, float b)
{
	return Vec2(a.x / b, a.y / b);
}

static Vec2 operator / (float a, const Vec2 & b)
{
	return Vec2(a / b.x, a / b.y);
}

static bool operator == (const Vec2 & a, const Vec2 & b)
{
	return a.x == b.x && a.y == b.y;
}

namespace Math {
	inline float Dot(const Vec2 & a, const Vec2 & b)
	{
		return a.x * b.x + a.y * b.y;
	}

	inline float Cross(const Vec2 & a, const Vec2 & b)
	{
		return a.x * b.y - a.y * b.x;
	}

	inline float LengthSqr(const Vec2 & v)
	{
		return Dot(v, v);
	}

	inline float Length(const Vec2 & v)
	{
		return std::sqrt(LengthSqr(v));
	}

	inline Vec2 Normal(const Vec2 v)
	{
		Vec2 r;
		auto l = Length(v);
		if (l != 0)
		{
			r.x = v.x / l;
			r.y = v.y / l;
		}
		return r;
	}

	inline float Lerp(float a, float b, float t)
	{
		return (b - a) * t + a;
	}

	inline Vec2 Lerp(const Vec2 & a, const Vec2 & b, float t)
	{
		return Vec2(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t));
	}

	inline Vec2 Lerp01(const Vec2 & a, const Vec2 & b, float t)
	{
		if (t < 0) t = 0;
		if (t > 1) t = 1;
		return Vec2(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t));
	}

	inline Vec2 LimitLength(const Vec2 & a, const Vec2 & b, float max)
	{
		auto l = Length(b - a);
		auto d = std::min(max, l);
		return Normal(b - a) * d + a;
	}

	inline float Random(float min, float max)
	{
		static std::minstd_rand mr;
		return std::normal_distribution(min, max)(mr);
	}

	inline int Random(int min, int max)
	{
		static std::random_device r;
		static std::minstd_rand mr(r());
		return std::uniform_int_distribution<size_t>(min, max)(mr);
	}

	inline bool IsContains(const Cir & cir, const Vec2 & p)
	{
		return LengthSqr(cir.mO - p) <= cir.mR * cir.mR;
	}

	inline bool IsContains(const Cir & a, const Cir & b)
	{
		return LengthSqr(a.mO - b.mO) <= (a.mR + b.mR) * (a.mR + b.mR);
	}

	inline Vec2 Beizer(const Vec2 & a, const Vec2 & b, const Vec2 & c, float t)
	{
		return Lerp(Lerp(a, b, t), Lerp(b, c, t), t);
	}

	inline Vec2 Beizer(const Vec2 & a, const Vec2 & b, const Vec2 & c, const Vec2 & d, float t)
	{
		return Beizer(Lerp(a, b, t), Lerp(b, c, t), Lerp(c, d, t), t);
	}

	inline void GenBeizer(const std::vector<Vec2> & src, std::vector<Vec2> & dst)
	{
		dst.emplace_back(src.front());
		for (auto i = 2; i != src.size(); ++i)
		{
			auto & a = src.at(i - 2);
			auto & b = src.at(i - 1);
			auto & c = src.at(i	   );
			auto ab = Lerp(a, b, 0.5f);
			auto cb = Lerp(c, b, 0.5f);
			auto d = b - Lerp(ab, cb, 0.5f);
			dst.emplace_back(ab + d);
			dst.emplace_back(b);
			dst.emplace_back(cb + d);
		}
		dst.emplace_back(src.back());
	}

	inline Vec2 LimitCoord(const Vec2 & range, const Cir & cir)
	{
		auto p = cir.mO;
		if (cir.mO.x < cir.mR) p.x = cir.mR;
		if (cir.mO.y < cir.mR) p.y = cir.mR;
		if (cir.mO.x > range.x - cir.mR) p.x = range.x - cir.mR;
		if (cir.mO.y > range.y - cir.mR) p.y = range.y - cir.mR;
		return p;
	}

	inline float ToAngle(const Vec2 & vec)
	{
		auto r = std::atan2(vec.y, vec.x);
		auto a = 180 / (float)std::_Pi *r;
		return vec.y < 0 ? 360 + a : a;
	}
};

struct Beizer {
	struct Segment {
		float s;
		float t;
		size_t a;
		size_t b;
	};
	std::vector<Vec2>	 mVecs;
	std::vector<Segment> mSegs;

	void InitBeizer(const std::vector<Vec2> & ls)
	{
		Math::GenBeizer(ls,mVecs);
		auto last = ls.size() - 1;

		auto sumLen = 0.0f;
		for (auto i = 0; i != last; ++i)
		{
			auto & a = ls.at(i	  );
			auto & b = ls.at(i + 1);
			sumLen += Math::Length(a - b);
		}

		auto baseA = (size_t)0;
		auto baseS = 0.0f;
		for (auto i = 0; i != last; ++i)
		{
			auto & a = ls.at(i	  );
			auto & b = ls.at(i + 1);
			auto l = Math::Length(a - b);
			Segment seg;

			seg.s = baseS;
			baseS += l / sumLen;
			seg.t = baseS;

			seg.a = baseA;
			baseA += (i == 0 || i == last - 1 ? 2 : 3);
			seg.b = baseA;

			mSegs.emplace_back(seg);
		}
	}

	Vec2 Calc(float t)
	{
		if		(t <= mSegs.front().s)
		{
			return mVecs.at(mSegs.front().a);
		}
		else if (t >= mSegs.back().t)
		{
			return mVecs.at(mSegs.back().b);
		}
		else
		{
			auto idx = 0;
			for (auto i = 0; i != mSegs.size(); ++i)
			{
				if (mSegs.at(i).s <= t &&
					mSegs.at(i).t >  t)
				{ idx = i; break; }
			}

			const auto & seg = mSegs.at(idx);
			t -=  seg.s;t /= (seg.t - seg.s);
			if (idx == 0 || idx == mSegs.size() - 1)
			{
				return Math::Beizer(mVecs.at(seg.a), mVecs.at(seg.a + 1), mVecs.at(seg.a + 2), t);
			}
			else
			{
				return Math::Beizer(mVecs.at(seg.a), mVecs.at(seg.a + 1), mVecs.at(seg.a + 2), mVecs.at(seg.a + 3), t);
			}
		}
	}
};