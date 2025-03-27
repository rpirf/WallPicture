#pragma once

namespace WP
{
	class Monitor
	{
	public:
		Monitor();

		unsigned long brightness() const;
		unsigned long minBrightness() const;
		unsigned long maxBrightness() const;

		void setBrightness(unsigned long);

	private:
		bool			_initialized = false;
		void*			_monitorHandle;
		unsigned long	_brightness;
		unsigned long	_minBrightness;
		unsigned long	_maxBrightness;
	};
}