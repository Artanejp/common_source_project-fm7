
#ifndef _CSP_QT_DRAWITEM_H_
#define _CSP_QT_DRAWITEM_H_
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QBrush>
#include <QString>
#include <QPixmap>

//QT_BEGIN_NAMESPACE
class CSP_DrawItem : public QImage {
//	Q_OBJECT
protected:
	int _width;
	int _height;
	QBrush *default_brush;
	QBrush *fill_brush;
	QBrush *dot_brush;
	QBrush *toned_brush_light;
	QBrush *toned_brush_mid;
	QBrush *toned_brush_deep;
public:
	CSP_DrawItem(int width, int height, Format fmt = QImage::Format_RGBA8888);
	~CSP_DrawItem();
	QPainter painter;
	void drawFloppy5Inch(QColor &BGColor, QColor &FGColor, QColor &TextColor, float text_pt, QString text);
	void drawFloppy3_5Inch(QColor &BGColor, QColor &FGColor, QColor &TextColor, float text_pt, QString text);
	void drawCasetteTape(QColor &BGColor, QColor &FGColor, QColor &TextColor, float text_pt, QString text);

};
//QT_END_NAMESPACE
#endif /* _CSP_QT_DRAWITEM_H_ */
