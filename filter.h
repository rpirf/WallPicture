#pragma once

#include <QSize>
#include <QGuiApplication>
#include <QScreen>
#include <limits>

namespace WP
{
	class FrameFilter
	{
	public:
		FrameFilter()
			: _minWithToHeightRatio(0), _maxWithToHeightRatio(std::numeric_limits<double>::max())
		{}

		FrameFilter(double minWithToHeightRatio, double maxWithToHeightRatio)
			: _minWithToHeightRatio(minWithToHeightRatio),
			_maxWithToHeightRatio(maxWithToHeightRatio)
		{}

		bool operator==(const FrameFilter& item) const
		{
			return _minWithToHeightRatio == item._minWithToHeightRatio
				&& _maxWithToHeightRatio == item._maxWithToHeightRatio;
		}


		bool operator!=(const FrameFilter& item) const
		{
			return !operator==(item);
		}

		bool pass(const QSize& size) const
		{
			if (!size.isValid() || size.height() == 0)
				return false;

			double withToHeightRatio = size.width() / (double)size.height();
			return _minWithToHeightRatio <= withToHeightRatio
				&& withToHeightRatio < _maxWithToHeightRatio;
		}

		double minWithToHeightRatio() const
		{
			return _minWithToHeightRatio;
		}

		double maxWithToHeightRatio() const
		{
			return _maxWithToHeightRatio;
		}

		static FrameFilter suitableForMonitor()
		{
			auto screenSize = QGuiApplication::primaryScreen()->size();

			if (screenSize.width() >= screenSize.height())
				return FrameFilter(1.0, std::numeric_limits<double>::max());
			else
				return FrameFilter(0, 1.0);
		}

	private:
		double _minWithToHeightRatio;
		double _maxWithToHeightRatio;
	};
}