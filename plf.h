#pragma once

#include <vector>

namespace WP
{
	class PiecewiseLinearFunction
	{
	public:
		PiecewiseLinearFunction(const std::vector<double>& x, const std::vector<double>& y)
			: _x(x), _y(y)
		{}

		PiecewiseLinearFunction()
			: _x{ 0.0, 1.0 }, _y{ 0.0, 1.0 }	// simple linear function
		{}

		double operator()(double x)
		{
			if (_x.size() != _y.size() || _x.size() < 2)
				return 0;

			int i = 0;
			for (; i < _x.size() - 1; i++)
			{
				if (x >= _x[i] && x < _x[i + 1])
					break;
			}
			if (i == _x.size() - 1)
				return _y[_y.size() - 1];

			return _y[i] + (x - _x[i]) / (_x[i + 1] - _x[i]) * (_y[i + 1] - _y[i]);
		}

	private:
		std::vector<double> _x;
		std::vector<double> _y;
	};
}
