
#ifndef VEC2D_H
#define VEC2D_H


#include <cmath>

//--二次元ベクトル--
struct vec2d
{
	double x, y;

	vec2d() : x(0.0), y(0.0) {}
	vec2d(double x_, double y_) : x(x_), y(y_) {}

	//長さを求める
	double length(){ return sqrt(x*x + y*y); }
	//二乗長さを求める
	double lengthsq(){ return x*x + y*y; }
	//直交ベクトルの一つを求める
	vec2d perp(){ return vec2d(-y, x); }
	//正規化ベクトルを求める
	vec2d norm(){ double L = length(); return vec2d(x / L, y / L); }
	//ベクトルの回転
	vec2d rotate(double t){
		return vec2d(x*cos(t) - y*sin(t),
			x*sin(t) + y*cos(t));
	}

	//正規化
	void normalize(){ *this /= this->length(); }

	//演算子のオーバーロード（四則演算を可能にする）
	vec2d operator+(const vec2d& v)
	{
		return vec2d(x + v.x, y + v.y);
	}

	vec2d operator-(const vec2d& v)
	{
		return vec2d(x - v.x, y - v.y);
	}

	vec2d operator*(const double& a)
	{
		return vec2d(a*x, a*y);
	}

	vec2d operator/(const double& a)
	{
		return vec2d(x / a, y / a);
	}

	vec2d& operator+=(const vec2d& v)
	{
		*this = *this + v;
		return *this;
	}

	vec2d& operator-=(const vec2d& v)
	{
		*this = *this - v;
		return *this;
	}

	vec2d& operator*=(const double& a)
	{
		*this = *this * a;
		return *this;
	}

	vec2d& operator/=(const double& a)
	{
		*this = *this / a;
		return *this;
	}

	//vec2d同士の*は内積
	double operator*(const vec2d& v)
	{
		return x*v.x + y*v.y;
	}
};

#endif
