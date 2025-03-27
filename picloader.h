#pragma once
#include <QPixmap>

#include "filter.h"
#include "settings.h"

namespace WP
{
	struct Picture
	{
		Picture(const QPixmap& pixmap_, const QString& label_)
			: pixmap(pixmap_), label(label_)
		{}
		Picture()
		{}

		QPixmap pixmap;
		QString label;
	};

	class PictureLoader
	{
	public:
		PictureLoader(const QString& libraryPath,
			const FrameFilter& filter = FrameFilter(), Order order = Order::Sequential,
			const std::function<void(int)>* progressFn = nullptr,
			const QSize& maxScreenSize = QSize(1920, 1080));

		void setLibrary(const QString& libraryPath);
		void setFilter(const FrameFilter& filter,
			const std::function<void(int)>* progressFn = nullptr);
		void setOrder(Order order);

		Picture loadNext();
		Picture loadPrev();

	private:
		Picture picture(int index) const;

		QString		_libraryPath;
		FrameFilter _filter;
		Order		_order;

		QStringList		_sequantialPaths;
		QStringList		_randomPaths;
		QStringList*	_paths;
		QSize			_maxScreenSize;
		int				_currentIndex = 0;
	};
}
