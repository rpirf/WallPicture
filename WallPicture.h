#pragma once

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QDeadlineTimer>
#include <memory>

namespace WP
{
    class Monitor;
    class PictureLoader;
    class ScaledPixmap;
    class LightSensor;
    class PiecewiseLinearFunction;
    struct Settings;
    struct Picture;
    struct TimerSettings;

    class WallPicture : public QWidget
    {
        Q_OBJECT

    public:
        WallPicture(const std::function<void(int)>* progressFn = nullptr, QWidget* parent = nullptr);
        ~WallPicture();

    protected:
        void keyReleaseEvent(QKeyEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void resizeEvent(QResizeEvent* event);
        bool eventFilter(QObject* obj, QEvent* event);

    private slots:
        void sleep();
        void showSettings();
        void onFullScreen();
        void onTimer();
        void showNext();
        void showPrev();
        void sensorReady();
        void updateBrightness();

    private:
        QPushButton* createButton(const QPixmap& pixmap, const QSize& size);
        void startTimer(const TimerSettings& timerSettings);
        void setControlsVisible(bool value);
        void setSleepingMode(bool value);
        bool isFullScreen() const;
        void setFullScreen(bool);
        void placeControls(const QSize& windowSize);
        void showPicture(const Picture&);
        void applySettings(const Settings& settings);
        int  remainingMs(const TimerSettings& timerSettings) const;
        QPixmap changePixmapBrightness(const QPixmap& pixmap, double multiplier) const;
        inline void convert(uchar* value, float multiplier) const
        {
            *value = *value * multiplier;
        };

        // active objects
        std::unique_ptr<Monitor> _monitor;
        std::unique_ptr<QTimer> _timerSleep;
        std::unique_ptr<QTimer> _timerNext;
        std::unique_ptr<QTimer> _timerBrightness;
        std::unique_ptr <PictureLoader> _picLoader;
        LightSensor* _sensor;
        QDeadlineTimer _deadLineTimer;

        // gui elements
        ScaledPixmap*   _picture;
        QPushButton*    _settings;
        QPushButton*    _showFullScreen;
        QPushButton*    _next;
        QPushButton*    _prev;
        QWidget*        _signature;
        QLabel*         _label;

        // current status or settings
        unsigned long _initialBrightness;
        QPixmap _pixmap;
        std::unique_ptr<Settings> _commonSettings;
        bool _isSleepingMode = true;
        bool _isFullScreen = true;
        double _lastBrightness = 1.0;
    };
}
