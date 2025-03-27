#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include "settings.h"

namespace WP
{
	class SettingsDialog : public QDialog
	{
		Q_OBJECT

	public:
		SettingsDialog(QWidget* parent = nullptr, bool hasBrightnessSensor = true,
			const QSize& screenSize = QSize(1920, 1080),
			const Settings& settings = Settings(),
			Qt::WindowFlags f = Qt::WindowFlags());

		void setSettings(const Settings& settings);
		Settings currentSettings() const;

	private slots:
		void selectLibrary();
		void changedAutoBrightness(bool checked);
		void changedTimer(int);
		void onOk(QAbstractButton*);

	private:
		QLineEdit* _libPath;
		QComboBox* _order;
		QComboBox* _filter;
		QCheckBox* _autoBrightness = nullptr;
		QComboBox* _brighnessPreset = nullptr;
		QComboBox* _timer;
		QComboBox* _time;

		std::vector<FrameFilter>				_filters;
		std::vector<PiecewiseLinearFunction>	_presets;
	};
}
