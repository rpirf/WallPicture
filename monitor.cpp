#include "monitor.h"

#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>

#pragma comment(lib, "Dxva2.lib")

using namespace WP;

HANDLE monitor()
{
	DWORD NumberOfPhysicalMonitors{ 0 };
	PHYSICAL_MONITOR* PhysicalMonitors{ nullptr };

	HMONITOR hMonitor{ MonitorFromWindow(GetTopWindow(nullptr), NULL) };

	if (!hMonitor || hMonitor == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &NumberOfPhysicalMonitors))
		return INVALID_HANDLE_VALUE;

	PhysicalMonitors = static_cast<PHYSICAL_MONITOR*>(malloc(
		NumberOfPhysicalMonitors * sizeof(PHYSICAL_MONITOR)
	));

	if (!GetPhysicalMonitorsFromHMONITOR(hMonitor,
		NumberOfPhysicalMonitors,
		PhysicalMonitors))
	{
		free(PhysicalMonitors);
		return INVALID_HANDLE_VALUE;
	}

	return PhysicalMonitors->hPhysicalMonitor;
}

Monitor::Monitor()
{
	_monitorHandle = monitor();

	constexpr int tryMaxCount = 5;
	for (int i = 0; i < tryMaxCount; i++)
	{
		GetMonitorBrightness(_monitorHandle, &_minBrightness, &_brightness, &_maxBrightness);
		if (_minBrightness < _maxBrightness)
		{
			_initialized = true;
			break;
		}
	}
}

unsigned long Monitor::brightness() const
{
	return _brightness;
}

unsigned long Monitor::minBrightness() const
{
	return _minBrightness;
}

unsigned long Monitor::maxBrightness() const
{
	return _maxBrightness;
}

void Monitor::setBrightness(unsigned long value)
{
	if (!_initialized)
		return;

	if (_brightness == value)
		return;

	if (value < _minBrightness || value > _maxBrightness)
		return;

	SetMonitorBrightness(_monitorHandle, value);
	_brightness = value;
}