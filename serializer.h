#pragma once

#include "settings.h"

#include <QIODevice>
#include <QSettings>
#include <QBuffer>

namespace WP
{
	class SettingsSerializer
	{
	public:
		static void read(QIODevice* pFile, Settings* settings)
		{
			settings->libraryPath = readString(pFile);
			pFile->read((char*)&settings->order, sizeof(settings->order));

			double minWithToHeightRatio = 0;
			double maxWithToHeightRatio = 0;
			pFile->read((char*)&minWithToHeightRatio, sizeof(minWithToHeightRatio));
			pFile->read((char*)&maxWithToHeightRatio, sizeof(maxWithToHeightRatio));
			settings->filter = FrameFilter(minWithToHeightRatio, maxWithToHeightRatio);

			pFile->read((char*)&settings->timer.triggerType,
				sizeof(settings->timer.triggerType));
			pFile->read((char*)&settings->timer.msTimerInterval,
				sizeof(settings->timer.msTimerInterval));
			pFile->read((char*)&settings->timer.msSinceMidnight,
				sizeof(settings->timer.msSinceMidnight));

			pFile->read((char*)&settings->brightness.autoBrightness,
				sizeof(settings->brightness.autoBrightness));
			settings->brightness.presetName = readString(pFile);
		}

		static void write(QIODevice* pFile, const Settings* settings)
		{
			writeString(pFile, settings->libraryPath);
			pFile->write((char*)&settings->order, sizeof(settings->order));

			double minWithToHeightRatio = settings->filter.minWithToHeightRatio();
			double maxWithToHeightRatio = settings->filter.maxWithToHeightRatio();
			pFile->write((char*)&minWithToHeightRatio, sizeof(minWithToHeightRatio));
			pFile->write((char*)&maxWithToHeightRatio, sizeof(maxWithToHeightRatio));

			pFile->write((char*)&settings->timer.triggerType,
				sizeof(settings->timer.triggerType));
			pFile->write((char*)&settings->timer.msTimerInterval,
				sizeof(settings->timer.msTimerInterval));
			pFile->write((char*)&settings->timer.msSinceMidnight,
				sizeof(settings->timer.msSinceMidnight));

			pFile->write((char*)&settings->brightness.autoBrightness,
				sizeof(settings->brightness.autoBrightness));
			writeString(pFile, settings->brightness.presetName);
		}

	private:
		static QString readString(QIODevice* file)
		{
			int32_t nStrLen = 0;
			file->read((char*)&nStrLen, sizeof(int));
			if (nStrLen <= 0)
				return "";

			QChar* aBuf = new QChar[nStrLen];
			file->read((char*)aBuf, nStrLen * sizeof(QChar));
			QString result(aBuf, nStrLen);
			delete[] aBuf;

			return result;
		}

		static bool writeString(QIODevice* file, const QString& str)
		{
			int32_t nStrLen = (int32_t)str.size();
			if (file->write((char*)&nStrLen, sizeof(int)) < 0)
				return false;
			if (file->write((char*)str.data(), nStrLen * sizeof(QChar)) < 0)
				return false;
			return true;
		}
	};

	class SettigsIO
	{
	public:
		static Settings loadlastSettings()
		{
			QSettings qSet("WP Inc.", "WallPicture");
			auto byteArray = qSet.value("serializerValue").toByteArray();
			if (byteArray.isEmpty())
				return Settings();

			QBuffer buffer(&byteArray);
			buffer.open(QIODevice::ReadOnly);

			Settings settings;
			SettingsSerializer::read(&buffer, &settings);
			return settings;
		}

		static void saveLastSettings(const Settings& settings)
		{
			QByteArray byteArray;
			QBuffer buffer(&byteArray);
			buffer.open(QIODevice::WriteOnly);
			SettingsSerializer::write(&buffer, &settings);

			QSettings qSet("WP Inc.", "WallPicture");
			qSet.setValue("serializerValue", byteArray);
		}
	};
}
