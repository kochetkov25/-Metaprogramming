#include <iostream>
#include <tuple>

/******meta class intList******/
template<int ... Types>
struct IntList;

template<int H, int ... T>
struct IntList<H, T...> {
	static const int Head = H;
	using Tail = IntList<T...>;
};
/*using to beak recursion*/
template<>
struct IntList<> {};
/******************************/


/******meta function Length******/
template<typename IL>
struct Length
{
	static int const value = 1 + Length<typename IL::Tail>::value;
};
/*using to break recursion*/
template<>
struct Length<IntList<>>
{
	static const int value = 0;
};
/********************************/


/******meta function IntCons******/
template<int H, typename IL>
struct IntCons;

template<int H, int... Values>
struct IntCons<H, IntList<Values ...>>
{
	using type = IntList<H, Values...>;
};
/*********************************/


/******meta function Generate******/
template<int N, int start = 0>
struct Generate
{
	using type = typename IntCons<start, typename Generate< N - 1, start + 1>::type>::type;
};
/*using to break recursion*/
template<int N>
struct Generate<0, N>
{
	using type = IntList<>;
};
/**********************************/


/******function apply******/
template<typename F, typename TUP, int ... args>
auto apply_impl(F f, TUP tup, IntList<args...>)->decltype(f(std::get<args>(tup)...))
{
	return f(std::get<args>(tup) ...);
}

template<typename F, typename ... Args>
auto apply(F f, std::tuple<Args...> tup)->decltype(apply_impl(f, tup, typename Generate<sizeof...(Args)>::type()))
{
	return apply_impl(f, tup, typename Generate<sizeof...(Args)>::type());
}
/**************************/


/******meta funct plus******/
template<int a, int b>
struct Plus
{
	static int const value = a + b;
};
/**************************/


/******meta func minus******/
template<int a, int b>
struct Minus
{
	static int const value = a - b;
};
/**************************/


/******meta function Zip******/
template<typename IL1, typename IL2, template <int, int> class Bin>
struct Zip
{
	using type = typename IntCons<Bin<IL1::Head, IL2::Head>::value, typename Zip<typename IL1::Tail, typename IL2::Tail, Bin>::type>::type;
};

template<template <int, int> class Bin>
struct Zip<IntList<>, IntList<>, Bin>
{
	using type = IntList<>;
};
/*****************************/


template<typename IL>
class Quantity
{
public:
	/*default ctor*/
	Quantity() = default;
	/*ctor*/
	explicit Quantity(double val) :_value(val) {}

	/*template multiplication operator*/
	template<typename IL1>
	Quantity<typename Zip<IL, IL1, Plus>::type> operator*(const Quantity<IL1>& other)
	{
		return Quantity< typename Zip<IL, IL1, Plus>::type >(_value * other._value);
	}
	/*multiplication operator*/
	Quantity<IL> operator*(const double val)
	{
		return Quantity<IL>(_value * val);
	}

	/*template division operator*/
	template<typename IL1>
	Quantity<typename Zip<IL, IL1, Minus>::type> operator/(const Quantity<IL1>& other)
	{
		return Quantity< typename Zip<IL, IL1, Minus>::type >(_value / other._value);
	}
	/*division operator*/
	Quantity<IL> operator/(const double val)
	{
		return Quantity<IL>(_value / val);
	}

	/*plus operator*/
	Quantity<IL> operator+(const Quantity<IL>& other)
	{
		return Quantity<IL>(_value + other._value);
	}

	/*minus operator*/
	Quantity<IL> operator-(const Quantity<IL>& other)
	{
		return Quantity<IL>(_value - other._value);
	}

	const double value() const
	{
		return _value;
	}

	double _value = 0;
};

/*multiplication operator (left double)*/
template<typename IL>
Quantity<IL> operator*(const double val, const Quantity<IL>& other)
{
	return Quantity<IL>(other._value * val);
}

/*division operator (left double)*/
template<typename IL>
Quantity<typename Zip<IntList<0, 0, 0, 0, 0, 0, 0>, IL, Minus>::type> operator/(const double val, const Quantity<IL>& other)
{
	return Quantity<typename Zip<IntList<0, 0, 0, 0, 0, 0, 0>, IL, Minus>::type>(val / other._value);
}

/*Dimension template*/
template<int m = 0, int kg = 0, int s = 0, int A = 0, int K = 0, int mol = 0, int cd = 0>
using Dimension = IntList<m, kg, s, A, K, mol, cd>;

int main()
{
	/*exmp IntList*/
	using primes = IntList<2, 3, 5, 7, 11, 13>;

	/*exmp Length*/
	constexpr size_t len = Length<primes>::value;
	std::cout << "Length: " << len << std::endl;

	/*exmp IntCons and Generate*/
	using L1 = IntList<2, 3, 4>;
	using L2 = IntCons<1, L1>::type;
	using L3 = Generate<5>::type;

	/*exmp apply*/
	auto f = [](int x, double y, double z) { return x + y + z; };
	auto t = std::make_tuple(30, 5.0, 1.6);
	auto res = apply(f, t);
	std::cout << "Res: " << res << std::endl;

	/*exmp Quantity*/
	using NumberQ = Quantity<Dimension<>>;           // number without dimension
	using LengthQ = Quantity<Dimension<1>>;          // meters
	using MassQ = Quantity<Dimension<0, 1>>;       // kilograms
	using TimeQ = Quantity<Dimension<0, 0, 1>>;    // seconds
	using VelocityQ = Quantity<Dimension<1, 0, -1>>;   // meters per second
	using AccelQ = Quantity<Dimension<1, 0, -2>>;   // acceleration, meters per second squared
	using ForceQ = Quantity<Dimension<1, 1, -2>>;   // force, Newtons

	AccelQ    a{ 9.8 };
	MassQ     m{ 80 };
	ForceQ    F1 = m * a;
	std::cout << "F1= " << F1.value() << " Newtons" << std::endl;

	return 0;
}