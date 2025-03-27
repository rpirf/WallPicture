#include "lightSensor.h"

#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QDir>
#include <QTimer>

using namespace WP;

constexpr double maxValueNonInitialized = 100.0;

LightSensor::LightSensor(QObject* parent)
    : QObject(parent)
{
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty())
        return;

    QCamera* camera = new QCamera(cameras[0], this);

    if (!camera->isCaptureModeSupported(QCamera::CaptureStillImage))
        return;

    camera->setCaptureMode(QCamera::CaptureStillImage);
    camera->start();

    _imageCapture = new QCameraImageCapture(camera, this);
    auto resolutions = _imageCapture->supportedResolutions();
    if (resolutions.isEmpty())
        return;

    // set smallest resolution
    QImageEncoderSettings settings = _imageCapture->encodingSettings();
    settings.setResolution(resolutions[0]);
    _imageCapture->setEncodingSettings(settings);

    _capturePath = QDir::homePath() + "/LightSensorSample.jpg";
    _captureTimer = new QTimer(this);
    _captureTimer->setInterval(100);
    QObject::connect(_captureTimer, &QTimer::timeout, this, &LightSensor::capture);
    QObject::connect(_imageCapture, &QCameraImageCapture::imageCaptured,
        this, &LightSensor::captured);

    _brightness.resize(50);
    for (int i = 0; i < _brightness.size(); i++)
        _brightness.push_back(0);

    _initialized = true;
}

bool LightSensor::isActive() const
{
    return _initialized;
}

void LightSensor::startCalibration()
{
    if (!_initialized)
        return;

    _calibrated = false;
    _captureTimer->start();
}

void LightSensor::capture()
{
    _imageCapture->capture(_capturePath);
}

void LightSensor::captured(int id, const QImage& preview)
{
    long long int sum = 0;
    for (int x = 0; x < preview.width(); x++)
    {
        for (int y = 0; y < preview.height(); y++)
            sum += qGray(preview.pixel(x, y));
    }
    double brightness = sum / (double)preview.width() / preview.height();
    _brightness.push_back(brightness);

    static int counter = 0;
    if (!_calibrated)
    {
        counter++;
        if (counter > _brightness.size())
        {
            _calibrated = true;
            counter = 0;

            // Last value is correct candidate to _maxBrightness.
            // Why?
            // Because camera has an auto brightness adjustment (auto iso),
            // and it performs an adjustment due ~5 second (our calibration time),
            // when camera first turned on.
            // Only one rule: you must start calibration only if you have full light power
            // (for example sunny day or light bulb on).
            _maxBrightness = brightness;

            emit calibrationFinished();
        }
    }
    int a = 0;
}

double LightSensor::currentValue() const
{
    if (_brightness.size() == 0)
        return maxValueNonInitialized;

    double sum = 0;
    for (int i = 0; i < _brightness.size(); i++)
        sum += _brightness[i];

    double value =  sum / _brightness.size();
    return std::min<double>(value, _maxBrightness);
}

double LightSensor::minValue() const
{
    return 0;
}

double LightSensor::maxValue() const
{
    if (_initialized)
        return _maxBrightness;
    else
        return maxValueNonInitialized;
}