#pragma once

#include <QString>
#include <QCoreApplication>

#include "plf.h"
#include "filter.h"

namespace WP
{
	enum class Order
	{
		Sequential,
		Random
	};

	struct TimerSettings
	{
		TimerSettings()
			: triggerType(TriggerType::DailyAwakening),
			msTimerInterval(60 * 60 * 1000),
			msSinceMidnight(3 * 60 * 60 * 1000)
		{}
		bool operator==(const TimerSettings&) const = default;

		enum class TriggerType
		{
			SimpleInterval,
			DailyAwakening
		};

		TriggerType	triggerType;
		int			msTimerInterval;		// for TriggerType::SimpleInterval (value in msec)
		int			msSinceMidnight;		// for TriggerType::DailyAwakening (value in msec)
	};

	struct BrightnessSettings
	{
		BrightnessSettings()
			: autoBrightness(true),
			brightnessPreset(PiecewiseLinearFunction(
				{ 0.0, 0.4, 0.6, 0.9, 1.0 },
				{ 0.0, 0.6, 0.9, 1.0, 1.0 })),
			presetName("Перевернутая гипербола")
		{}

		bool					autoBrightness;
		PiecewiseLinearFunction	brightnessPreset;
		QString					presetName;
	};

	struct Settings
	{
		QString				libraryPath = QCoreApplication::applicationDirPath() + "/pictures";
		Order				order = Order::Sequential;
		FrameFilter			filter = FrameFilter::suitableForMonitor();
		TimerSettings		timer;
		BrightnessSettings	brightness;
	};
}
