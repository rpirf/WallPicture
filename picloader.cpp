#include "picloader.h"
#include <QDir>
#include <QDirIterator>
#include <QCoreApplication>
#include <QTextCodec>
#include <QRandomGenerator>
#include <QImageReader>
#include <QScreen>

using namespace WP;

QStringList allPicturePaths(const QString& rootDir)
{
	QStringList result;

	QDirIterator it(rootDir, QStringList() << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp",
		QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext())
		result << it.next();

	return result;
}

QSize jpegSize(const QString& path)
{
	static QTextCodec* codec = QTextCodec::codecForName("CP1251");
	std::string path1 =  codec->fromUnicode(path).data();
	const char* strPath = path1.data();

	FILE* image = fopen(strPath, "rb");  // open JPEG image file
	if (!image)
		return QSize(0, 0);

	int width = 0, height = 0;
	int i = 0;
	unsigned char* data;

	fseek(image, 0, SEEK_END);
	int size = ftell(image);
	fseek(image, 0, SEEK_SET);
	data = (unsigned char*)malloc(size);
	fread(data, 1, size, image);
	/* verify valid JPEG header */
	if (data[i] == 0xFF && data[i + 1] == 0xD8 && data[i + 2] == 0xFF && data[i + 3] == 0xE0)
	{
		i += 4;
		/* Check for null terminated JFIF */
		if (data[i + 2] == 'J' && data[i + 3] == 'F' && data[i + 4] == 'I'
			&& data[i + 5] == 'F' && data[i + 6] == 0x00)
		{
			while (i < size)
			{
				i++;
				if (data[i] == 0xFF)
				{
					if (data[i + 1] == 0xC0)
					{
						height = data[i + 5] * 256 + data[i + 6];
						width = data[i + 7] * 256 + data[i + 8];
						break;
					}
				}
			}
		}
	}
	fclose(image);
	free(data);

	if (width == 0 && height == 0)
	{
		// very slow
		QSize size =  QPixmap(path).size();
		width = size.width();
		height = size.height();
	}

	return QSize(width, height);
}

QSize size(const QString& path)
{
	QString suffix = QFileInfo(path).suffix().toUpper();
	if (suffix == "JPG" || suffix == "JPEG")
		return jpegSize(path);
	else
		return QPixmap(path).size();
}

QStringList filter(const QStringList& list, const FrameFilter& filter,
	const std::function<void(int)>* progressFn = nullptr)
{
	QStringList result;
	int lastProgress = -1;
	for (int i = 0; i < list.size(); i++)
	{
		if (filter.pass(size(list[i])))
			result << list[i];

		if (progressFn)
		{
			int progress = 100 * i / list.size();
			if (progress != lastProgress)
			{
				(*progressFn)(100 * i / list.size());
				lastProgress = progress;
			}
		}
	}
	return result;
}

QStringList sortRandom(const QStringList& items)
{
	QRandomGenerator generator(QTime::currentTime().msec());
	int size = items.size();
	std::vector<int> indexes(size);

	for (int i = 0; i < size; i++)
		indexes[i] = i;

	for (int i = 0; i < size; i++)
	{
		int j = generator.generate() % size;

		int str = indexes[i];
		indexes[i] = indexes[j];
		indexes[j] = str;
	}

	QStringList result;
	result.reserve(size);
	for (int i = 0; i < size; i++)
		result.push_back(items[indexes[i]]);

	return std::move(result);
}

PictureLoader::PictureLoader(const QString& libraryPath, const FrameFilter& filter, Order order,
	const std::function<void(int)>* progressFn,
	const QSize& maxScreenSize)
	: _libraryPath(libraryPath),
	_maxScreenSize(maxScreenSize)
{
	setFilter(filter, progressFn);
	setOrder(order);
}

void PictureLoader::setLibrary(const QString& libraryPath)
{
	_libraryPath = libraryPath;
	setFilter(_filter);
	setOrder(_order);
}

void PictureLoader::setFilter(const FrameFilter& filter,
	const std::function<void(int)>* progressFn)
{
	_filter = filter;
	_sequantialPaths = allPicturePaths(_libraryPath);
	_sequantialPaths = ::filter(_sequantialPaths, _filter, progressFn);
	_randomPaths = sortRandom(_sequantialPaths);
}

void PictureLoader::setOrder(Order order)
{
	_order = order;
	_paths = (_order == Order::Random) ? &_randomPaths : &_sequantialPaths;
}

Picture PictureLoader::loadNext()
{
	if (!_paths || _currentIndex > _paths->size() || _currentIndex < 0 || _paths->size() == 0)
		return Picture();

	_currentIndex++;
	if (_currentIndex >= _paths->size())
		_currentIndex = 0;
	return picture(_currentIndex);
}

Picture PictureLoader::loadPrev()
{
	if (!_paths || _currentIndex > _paths->size() || _currentIndex < 0 || _paths->size() == 0)
		return Picture();

	_currentIndex--;
	if (_currentIndex < 0)
		_currentIndex = _paths->size() - 1;
	return picture(_currentIndex);
}

Picture PictureLoader::picture(int index) const
{
	auto path = (*_paths)[_currentIndex];
	auto pixmap = QPixmap(path);
	if (pixmap.width() > _maxScreenSize.width() || pixmap.height() > _maxScreenSize.height())
		pixmap = pixmap.scaled(_maxScreenSize, Qt::KeepAspectRatio);

	return Picture(pixmap, QFileInfo(path).completeBaseName());
}