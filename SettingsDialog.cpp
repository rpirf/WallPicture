#include "SettingsDialog.h"
#include "style.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDirIterator>

using namespace WP;

SettingsDialog::SettingsDialog(QWidget* parent, bool hasBrightnessSensor,
	const QSize& screenSize, const Settings& settings, Qt::WindowFlags f)
	: QDialog(parent, f)
{
	setStyleSheet(dialogStyle());

	auto layout = new QGridLayout();
	setLayout(layout);

	auto libText = new QLabel("Библиотека");
	layout->addWidget(libText, 0, 0);

	QHBoxLayout* hbLib = new QHBoxLayout;
	_libPath = new QLineEdit();
	auto select = new QPushButton("Выбрать...");
	hbLib->addWidget(_libPath);
	hbLib->addWidget(select);
	layout->addLayout(hbLib, 0, 1);
	connect(select, &QAbstractButton::clicked,
		this, &SettingsDialog::selectLibrary);

	auto orderText = new QLabel("Порядок воспроизведения");
	layout->addWidget(orderText, 1, 0);

	_order = new QComboBox();
	_order->addItem("последовательный");
	_order->addItem("случайный");
	layout->addWidget(_order, 1, 1);

	auto filterText = new QLabel("Показывать");
	layout->addWidget(filterText, 2, 0);

	_filter = new QComboBox();

	_filter->addItem("все изображения");
	_filters.emplace_back();

	_filter->addItem("только изобр-я, согласующ. с ориент. монитора");
	if (screenSize.width() >= screenSize.height())
		_filters.emplace_back(1.0, std::numeric_limits<double>::max());
	else
		_filters.emplace_back(0, 1.0);

	_filter->addItem("только изобр-я, согласующ. с соотнош. сторон монитора");
	double withToHeightRatio = screenSize.width() / (double)screenSize.height();
	_filters.emplace_back(withToHeightRatio * 0.8, withToHeightRatio * 1.2);

	layout->addWidget(_filter, 2, 1);

	auto timerText = new QLabel("Сменять изображение");
	layout->addWidget(timerText, 3, 0);

	_timer = new QComboBox();
	_timer->addItem("раз в минуту", QVariant(60 * 1000));
	_timer->addItem("раз в 2 минуты", QVariant(2 * 60 * 1000));
	_timer->addItem("раз в 10 минут", QVariant(10 * 60 * 1000));
	_timer->addItem("раз в час", QVariant(60 * 60 * 1000));
	_timer->addItem("раз в 2 часа", QVariant(2 * 60 * 60 * 1000));
	_timer->addItem("раз в 12 часов", QVariant(12 * 60 * 60 * 1000));
	_timer->addItem("раз в сутки");
	layout->addWidget(_timer, 3, 1);

	_time = new QComboBox();
	_time->addItem("в полночь");
	_time->addItem("в час ночи");
	_time->addItem("в 2 часа ночи");
	_time->addItem("в 3 часа ночи");
	_time->addItem("в 4 часа ночи");
	_time->addItem("в 5 часов утра");
	_time->addItem("в 6 часов утра");
	_time->addItem("в 7 часов утра");
	_time->addItem("в 8 часов утра");
	_time->addItem("в 9 часов утра");
	_time->addItem("в 10 часов утра");
	_time->addItem("в 11 часов утра");
	_time->addItem("в 12 часов");
	_time->addItem("в 13 часов");
	_time->addItem("в 14 часов");
	_time->addItem("в 15 часов");
	_time->addItem("в 16 часов");
	_time->addItem("в 17 часов");
	_time->addItem("в 18 часов");
	_time->addItem("в 19 часов");
	_time->addItem("в 20 часов");
	_time->addItem("в 21 час");
	_time->addItem("в 22 часа");
	_time->addItem("в 23 часа");

	connect(_timer, SIGNAL(currentIndexChanged(int)), this, SLOT(changedTimer(int)));

	QHBoxLayout* hbox = new QHBoxLayout;
	hbox->addWidget(_timer);
	hbox->addWidget(_time);
	layout->addLayout(hbox, 3, 1);

	if (hasBrightnessSensor)
	{
		auto frame = new QFrame();
		frame->setFrameShape(QFrame::HLine);
		layout->addWidget(frame, 4, 0, 1, 2);

		_autoBrightness = new QCheckBox("Автонастройка яркости экрана по датчику освещенности");
		layout->addWidget(_autoBrightness, 5, 0, 1, 2);

		auto brighnessText = new QLabel("Профиль яркости");
		layout->addWidget(brighnessText, 6, 0);

		_brighnessPreset = new QComboBox();

		_brighnessPreset->addItem("Линейный профиль");
		_presets.emplace_back(std::vector<double>({ 0.0, 1.0 }),
							  std::vector<double>({ 0.0, 1.0 }));
		_brighnessPreset->addItem("Перевернутая гипербола");
		_presets.emplace_back(std::vector<double>({ 0.0, 0.4, 0.6, 0.9, 1.0 }),
							  std::vector<double>({ 0.0, 0.6, 0.9, 1.0, 1.0 }));
		_brighnessPreset->addItem("Усиленная перевернутая гипербола");
		_presets.emplace_back(std::vector<double>({ 0.0, 0.2, 0.4, 0.6,  0.8, 1.0 }),
							  std::vector<double>({ 0.0, 0.5, 0.8, 0.92, 1.0, 1.0 }));
		_brighnessPreset->addItem("S-образный профиль");
		_presets.emplace_back(std::vector<double>({ 0.0, 0.2, 0.8, 1.0 }),
							  std::vector<double>({ 0.0, 0.07, 1.0, 1.0 }));

		layout->addWidget(_brighnessPreset, 6, 1);

		connect(_autoBrightness, &QAbstractButton::clicked,
			this, &SettingsDialog::changedAutoBrightness);

		auto frame2 = new QFrame();
		frame2->setFrameShape(QFrame::HLine);
		layout->addWidget(frame2, 7, 0, 1, 2);
	}

	auto buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton("Да", QDialogButtonBox::ActionRole);
	buttonBox->addButton("Отмена", QDialogButtonBox::RejectRole);
	connect(buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::onOk);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttonBox, 8, 0, 1, 2);

	setSettings(settings);
}

void SettingsDialog::selectLibrary()
{
	auto dirPath = QFileDialog::getExistingDirectory(
		this, "Выберите директорию с картинами", _libPath->text());
	if (!dirPath.isEmpty())
		_libPath->setText(dirPath);
}

void SettingsDialog::changedAutoBrightness(bool checked)
{
	_brighnessPreset->setEnabled(checked);
}

void SettingsDialog::changedTimer(int currentIndex)
{
	bool visible = (currentIndex == _timer->count() - 1);
	_time->setVisible(visible);
}

void SettingsDialog::onOk(QAbstractButton* button)
{
	if (button->text() != "Да")
		return;

	QFileInfo info(_libPath->text());

	bool isExistedDir = info.exists() && info.isDir();
	if (!isExistedDir)
	{
		QMessageBox::warning(nullptr, "Внимание!",
			"Неверно выбрана директория, содержащая библиотеку картин."
			" Указанной директории не существует.");
		return;
	}

	QDirIterator it(_libPath->text(), QStringList() << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp",
		QDir::Files, QDirIterator::Subdirectories);
	if (!it.hasNext())
	{
		QMessageBox::warning(nullptr, "Внимание!",
			"Неверно выбрана директория, содержащая библиотеку картин."
			" В указанной директории не найдено изображений (png, jpeg, bmp).");
		return;
	}

	accept();
}

void SettingsDialog::setSettings(const Settings& settings)
{
	_libPath->setText(settings.libraryPath);
	_order->setCurrentIndex((int)settings.order);

	auto iter = std::find(_filters.begin(), _filters.end(), settings.filter);
	if (iter != _filters.end())
		_filter->setCurrentIndex(std::distance(_filters.begin(), iter));
	else
		_filter->setCurrentIndex(0);

	if (settings.timer.triggerType == TimerSettings::TriggerType::DailyAwakening)
	{
		_timer->setCurrentIndex(_timer->count() - 1);
		_time->setVisible(true);
		_time->setCurrentIndex(settings.timer.msSinceMidnight / 60 / 60 / 1000);
	}
	else
	{
		int index = 0;
		for (; index < _timer->count() - 1; index++)
		{
			if (_timer->itemData(index).toInt() == settings.timer.msTimerInterval)
			{
				_timer->setCurrentIndex(index);
				break;
			}
		}
		if (index != _timer->count() - 1)
			_timer->setCurrentIndex(index);
		else
			_timer->setCurrentIndex(0);

		_time->setVisible(false);
		_time->setCurrentIndex(settings.timer.msSinceMidnight / 60 / 60 / 1000);
	}

	if (_autoBrightness)
	{
		_autoBrightness->setCheckState(
			settings.brightness.autoBrightness ? Qt::Checked : Qt::Unchecked);
	}

	if (_brighnessPreset)
	{
		bool presetIsFound = false;
		for (int i = 0; i < _brighnessPreset->count(); i++)
		{
			if (_brighnessPreset->itemText(i) == settings.brightness.presetName)
			{
				_brighnessPreset->setCurrentIndex(i);
				presetIsFound = true;
				break;
			}
		}
		if (!presetIsFound)
			_brighnessPreset->setCurrentIndex(0);

		_brighnessPreset->setEnabled(settings.brightness.autoBrightness);
	}
}

Settings SettingsDialog::currentSettings() const
{
	Settings result;

	result.libraryPath = _libPath->text();
	result.order = (WP::Order)_order->currentIndex();

	if (_filter->currentIndex() < _filters.size())
		result.filter = _filters[_filter->currentIndex()];

	if (_timer->currentIndex() == _timer->count() - 1)
	{
		result.timer.triggerType = TimerSettings::TriggerType::DailyAwakening;
		result.timer.msSinceMidnight = _time->currentIndex() * 60 * 60 * 1000;
	}
	else
	{
		result.timer.triggerType = TimerSettings::TriggerType::SimpleInterval;
		result.timer.msSinceMidnight = _time->currentIndex() * 60 * 60 * 1000;
		result.timer.msTimerInterval = _timer->currentData().toInt();
	}

	if (_brighnessPreset && _autoBrightness && _presets.size() > _brighnessPreset->currentIndex())
	{
		result.brightness.autoBrightness = _autoBrightness->isChecked();
		result.brightness.brightnessPreset = _presets[_brighnessPreset->currentIndex()];
		result.brightness.presetName = _brighnessPreset->currentText();
	}

	return result;
}