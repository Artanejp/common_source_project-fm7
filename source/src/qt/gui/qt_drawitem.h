
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

	void drawRectItem(int _x, int _y, int _w, int _h, QBrush *brush, QColor &_color1, QColor &_color2, Qt::PenStyle style = Qt::SolidLine);
	void drawCircleItem(int _x, int _y, int _r, int _begin, int _end, QBrush *brush, QColor &_color1, QColor &_color2, Qt::PenStyle style = Qt::SolidLine);
	void drawPolygonItem(QPointF _points[], int members, QBrush *brush, QColor &_color1, QColor &_color2, Qt::PenStyle style = Qt::SolidLine);
	void drawTextItem(int x, int y, int w, int h, QString &_str, QBrush *brush, QColor &_color1, QColor & _color2);
public:
	QPainter painter;
	CSP_DrawItem(int width, int height, Format fmt = QImage::Format_RGBA8888);
	~CSP_DrawItem();
	void clearCanvas(QColor &clColor);
	void drawFloppy5Inch(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &TextColor, float text_pt, QString text);
	void drawFloppy3_5Inch(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &FGColor3, QColor &TextColor, float text_pt, QString text);
	void drawCasetteTape(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &FGColor3, QColor &TextColor, float text_pt, QString text);
	void drawQuickDisk(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &TextColor, float text_pt, QString text);
	void drawCompactDisc(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &FGColor3, QColor &LabelColor, QColor &TextColor, float text_pt, QString text);
	void drawLaserDisc(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &LabelColor, QColor &TextColor, float text_pt, QString text);
};
//QT_END_NAMESPACE
#endif /* _CSP_QT_DRAWITEM_H_ */
