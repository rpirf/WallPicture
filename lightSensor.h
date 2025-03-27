#pragma once

#include "circlebuffer.h"
#include <QObject>

class QCameraImageCapture;
class QTimer;

namespace WP
{
	// This sensor is based on camera picture
	class LightSensor : public QObject
	{
		Q_OBJECT

	public:
		LightSensor(QObject* parent = nullptr);
		bool isActive() const;

		// Only one rule: you must start calibration only if you have full light power
		// (for example sunny day or light bulb on).
		void startCalibration();

		// you can call this function after calibrationFinished only
		double currentValue() const;
		double minValue() const;
		double maxValue() const;

	signals:
		void calibrationFinished();

	private slots:
		void capture();
		void captured(int id, const QImage& preview);

	private:
		bool					_initialized = false;
		bool					_calibrated = false;

		CircleBuffer<double>	_brightness;
		double					_maxBrightness = 0;

		QCameraImageCapture*	_imageCapture;
		QString					_capturePath;
		QTimer*					_captureTimer;
	};
}