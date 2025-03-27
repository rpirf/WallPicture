#include "WallPicture.h"
#include "monitor.h"
#include "picloader.h"
#include "scaledpixmap.h"
#include "lightSensor.h"
#include "plf.h"
#include "style.h"
#include "serializer.h"
#include "SettingsDialog.h"

#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QKeyEvent>
#include <QTimer>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>
#include <QDir>
#include <QDebug>
#include <QTime>

using namespace WP;

constexpr int littleBtnWidth = 120;
constexpr int littleBtnHeight = 120;
constexpr int offset = 20;

constexpr int btnWidth = 250;
constexpr int btnHeight = 350;

int startWindowWidth = 1400;
int startWindowHeight = 800;

WallPicture::WallPicture(const std::function<void(int)>* progressFn, QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    _commonSettings = std::make_unique<Settings>(WP::SettigsIO::loadlastSettings());

    auto orientation = screen()->orientation();
    if (orientation == Qt::PortraitOrientation || orientation == Qt::InvertedPortraitOrientation)
        std::swap(startWindowWidth, startWindowHeight);

    setGeometry(50, 50, startWindowWidth, startWindowHeight);
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, "black");
    pal.setColor(QPalette::Base, "black");
    this->setPalette(pal);

    _monitor = std::make_unique<Monitor>();
    _timerSleep = std::make_unique<QTimer>();
    _timerSleep->setInterval(5000);
    _timerSleep->setSingleShot(true);
    connect(_timerSleep.get(), SIGNAL(timeout()), this, SLOT(sleep()));

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    _initialBrightness = _monitor->brightness();

    _sensor = new LightSensor(this);
    connect(_sensor, &LightSensor::calibrationFinished, this, &WallPicture::sensorReady);

    _picture = new ScaledPixmap();
    _picture->setAttribute(Qt::WA_TransparentForMouseEvents);
    _picture->setPixmap(QPixmap(":/WallPicture/splash.PNG"));
    layout->addWidget(_picture);

    _settings = createButton(QPixmap(":/WallPicture/settings.png"),
        QSize(littleBtnWidth, littleBtnHeight));
    _settings->setGeometry(offset, offset, littleBtnWidth, littleBtnHeight);

    _showFullScreen = createButton(QPixmap(":/WallPicture/minimize.png"),
        QSize(littleBtnWidth, littleBtnHeight));
    _next = createButton(QPixmap(":/WallPicture/right.png"), QSize(150, 250));
    _prev = createButton(QPixmap(":/WallPicture/left.png"), QSize(150, 250));

    connect(_settings, &QAbstractButton::clicked, this, &WallPicture::showSettings);
    connect(_showFullScreen, &QAbstractButton::clicked, this, &WallPicture::onFullScreen);
    connect(_next, &QAbstractButton::clicked, this, &WallPicture::showNext);
    connect(_prev, &QAbstractButton::clicked, this, &WallPicture::showPrev);

    _signature = new QWidget(this);
    _signature->setStyleSheet(signatureStyle());
    _signature->installEventFilter(this);
    QHBoxLayout* hLayout = new QHBoxLayout();
    _signature->setLayout(hLayout);
    hLayout->addStretch();
    _label = new QLabel();
    hLayout->addWidget(_label);
    hLayout->addStretch();

    placeControls(QSize(startWindowWidth, startWindowHeight));

    _picLoader = std::make_unique<PictureLoader>(_commonSettings->libraryPath,
        _commonSettings->filter, _commonSettings->order, progressFn,
        screen()->size());

    _sensor->startCalibration();
    setMouseTracking(true);
    setSleepingMode(false);
    showNext();

    _timerNext = std::make_unique<QTimer>();
    connect(_timerNext.get(), &QTimer::timeout, this, &WallPicture::onTimer);
    startTimer(_commonSettings->timer);

    _timerBrightness = std::make_unique<QTimer>();
    _timerBrightness->setInterval(1000);
    connect(_timerBrightness.get(), &QTimer::timeout, this, &WallPicture::updateBrightness);
}

QPushButton* WallPicture::createButton(const QPixmap& pixmap, const QSize& size)
{
    auto button = new QPushButton(this);
    button->setIcon(pixmap);
    button->setFlat(true);
    button->setIconSize(size);
    button->installEventFilter(this);
    button->setStyleSheet(buttonStyle());
    return button;
}

void WallPicture::startTimer(const TimerSettings& timerSettings)
{
    if (timerSettings.triggerType == TimerSettings::TriggerType::SimpleInterval)
    {
        _timerNext->stop();
        _timerNext->setInterval(timerSettings.msTimerInterval);
        _timerNext->start();
    }
    else
    {
        _timerNext->stop();
        _deadLineTimer.setRemainingTime(remainingMs(timerSettings));
        _timerNext->setInterval(10000); // 10 second
        _timerNext->start();
    }
}

void WallPicture::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

void WallPicture::mouseMoveEvent(QMouseEvent* event)
{
    setSleepingMode(false);
}

void WallPicture::mousePressEvent(QMouseEvent* event)
{
    if (_isSleepingMode)
        setSleepingMode(false);
    else
        setSleepingMode(true);
}

bool WallPicture::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::HoverMove:
        setSleepingMode(false);
        break;
    case QEvent::TouchBegin:
        if (_isSleepingMode)
            setSleepingMode(false);
        else
            setSleepingMode(true);
    }
    return QWidget::eventFilter(obj, event);
}

void WallPicture::sleep()
{
    setSleepingMode(true);
}

QPixmap WallPicture::changePixmapBrightness(const QPixmap& pixmap, double multiplier) const
{
    QImage image = pixmap.toImage();
    auto format = image.format();
    if (format != QImage::Format_RGB32 && format != QImage::Format_ARGB32)
        image = image.convertToFormat(QImage::Format_RGB32);

    uchar* ptr = image.bits();
    int sizeInBytes = image.sizeInBytes();
    constexpr int pixelSizeInBytes = 4;
    for (int i = 0; i < sizeInBytes / pixelSizeInBytes; i++)
    {
        convert(ptr + i * pixelSizeInBytes, multiplier);
        convert(ptr + i * pixelSizeInBytes + 1, multiplier);
        convert(ptr + i * pixelSizeInBytes + 2, multiplier);
    }
    return QPixmap::fromImage(image);
}

void WallPicture::setSleepingMode(bool value)
{
    if (value == false)
    {
        _timerSleep->start();
        if (_isSleepingMode != value)
        {
            setControlsVisible(true);
            _monitor->setBrightness(_initialBrightness);
            _picture->setPixmap(_pixmap);

            qDebug() << "unsleep";
        }
    }
    else
    {
        if (_isSleepingMode != value)
        {
            setControlsVisible(false);
            updateBrightness();
            qDebug() << "sleep";
        }
    }
    _isSleepingMode = value;
}

void WallPicture::setControlsVisible(bool value)
{
    _settings->setVisible(value);
    _showFullScreen->setVisible(value);
    _next->setVisible(value);
    _prev->setVisible(value);
    _signature->setVisible(value);
    if (!value)
        QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
    else
        QGuiApplication::restoreOverrideCursor();
}

void WallPicture::resizeEvent(QResizeEvent* event)
{
    placeControls(event->size());
}

void WallPicture::placeControls(const QSize& windowSize)
{
    _showFullScreen->setGeometry(windowSize.width() - littleBtnWidth - offset, offset,
        littleBtnWidth, littleBtnHeight);
    _next->setGeometry(windowSize.width() - btnWidth, windowSize.height() / 2 - btnHeight / 2,
        btnWidth, btnHeight);
    _prev->setGeometry(0, windowSize.height() / 2 - btnHeight / 2,
        btnWidth, btnHeight);
    _signature->setGeometry(0, windowSize.height() - _signature->sizeHint().height(),
        windowSize.width(), _signature->sizeHint().height());
}

void WallPicture::onFullScreen()
{
    setFullScreen(!isFullScreen());
}

bool WallPicture::isFullScreen() const
{
    return _isFullScreen;
}

void WallPicture::setFullScreen(bool value)
{
    _isFullScreen = value;
    if (value)
    {
        _showFullScreen->setIcon(QPixmap(":/WallPicture/minimize.png"));
        showFullScreen();
        placeControls(screen()->size());
    }
    else
    {
        _showFullScreen->setIcon(QPixmap(":/WallPicture/maximize.png"));
        showNormal();
    }
}

void WallPicture::showSettings()
{
    setSleepingMode(false);
    _timerSleep->stop();

    WP::SettingsDialog dialog(nullptr, _sensor->isActive(), screen()->size(),
        *_commonSettings.get());
    int returnCode = dialog.exec();
    if (returnCode == QDialog::Rejected)
    {
        _timerSleep->start();
        return;
    }

    applySettings(dialog.currentSettings());
    _timerSleep->start();
}

void WallPicture::applySettings(const Settings& settings)
{
    bool isLoaderChanged = false;
    if (settings.libraryPath != _commonSettings->libraryPath)
    {
        std::function<void(int)> progressFn = [&](int percent)
            {
                QString str = QString("Загрузка: %1%").arg(percent);
                _label->setText(str);
                _timerSleep->stop();
                QCoreApplication::processEvents();
            };

        _picLoader.reset(new PictureLoader(
            settings.libraryPath, settings.filter, settings.order, &progressFn,
            screen()->size()));
        isLoaderChanged = true;
    }
    else
    {
        if (settings.filter != _commonSettings->filter)
        {
            _picLoader->setFilter(settings.filter);
            isLoaderChanged = true;
        }
        if (settings.order != _commonSettings->order)
        {
            _picLoader->setOrder(settings.order);
            isLoaderChanged = true;
        }
    }

    if (_commonSettings->brightness.autoBrightness != settings.brightness.autoBrightness)
    {
        if (!_commonSettings->brightness.autoBrightness)
            _monitor->setBrightness(_initialBrightness);
    }

    if (_commonSettings->timer != settings.timer)
        startTimer(settings.timer);

    _commonSettings.reset(new Settings(settings));
    WP::SettigsIO::saveLastSettings(*_commonSettings.get());

    if (isLoaderChanged)
        showNext();
}

int WallPicture::remainingMs(const TimerSettings& timerSettings) const
{
    time_t now_t = time(NULL);
    struct tm* now = localtime(&now_t);

    int msSinceModnight = (now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec) * 1000;
    if (msSinceModnight < timerSettings.msSinceMidnight)
        return timerSettings.msSinceMidnight - msSinceModnight;
    else
        return (24 * 3600 * 1000 - msSinceModnight) + timerSettings.msSinceMidnight;
}

void WallPicture::onTimer()
{
    if (_commonSettings->timer.triggerType == TimerSettings::TriggerType::SimpleInterval)
        showNext();
    else if (_deadLineTimer.hasExpired())
    {
        showNext();
        _deadLineTimer.setRemainingTime(remainingMs(_commonSettings->timer));
    }
}

void WallPicture::showNext()
{
    showPicture(_picLoader->loadNext());
}

void WallPicture::showPrev()
{
    showPicture(_picLoader->loadPrev());
}

void WallPicture::showPicture(const Picture& picture)
{
    _pixmap = picture.pixmap;

    if (_isSleepingMode && _sensor->isActive() && _commonSettings->brightness.autoBrightness)
    {
        double normBrightness = _sensor->currentValue() / _sensor->maxValue();
        normBrightness = _commonSettings->brightness.brightnessPreset(normBrightness);
        _picture->setPixmap(changePixmapBrightness(_pixmap, normBrightness));
        _lastBrightness = normBrightness;
    }
    else
        _picture->setPixmap(_pixmap);

    _label->setText(picture.label);
}

void WallPicture::sensorReady()
{
    _timerBrightness->start();
}

void WallPicture::updateBrightness()
{
    if (!_isSleepingMode || !_sensor->isActive()
        || !_commonSettings->brightness.autoBrightness)
    {
        return;
    }

    double normBrightness = _sensor->currentValue() / _sensor->maxValue();
    normBrightness = _commonSettings->brightness.brightnessPreset(normBrightness);
    _monitor->setBrightness(_initialBrightness * normBrightness);
    if (_lastBrightness != normBrightness)
    {
        _picture->setPixmap(changePixmapBrightness(_pixmap, normBrightness));
        _lastBrightness = normBrightness;
    }
}

WallPicture::~WallPicture()
{
    _monitor->setBrightness(_initialBrightness);
}
