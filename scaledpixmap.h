#pragma once

#include <QWidget>

namespace WP
{
	class ScaledPixmap : public QWidget
	{
		Q_OBJECT
	public:
		ScaledPixmap(QWidget* parent = nullptr);

		void	setPixmap(const QPixmap& pixmap);
		QPixmap	pixmap() const;
		QSize	sizeHint() const override;

	protected:
		void paintEvent(QPaintEvent* event) override;

	private:
		QPixmap _pixmap;
	};
}
