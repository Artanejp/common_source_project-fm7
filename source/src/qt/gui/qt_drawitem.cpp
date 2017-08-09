
#include "qt_drawitem.h"
#include <QPointF>

CSP_DrawItem::CSP_DrawItem(int width, int height, Format fmt) : QImage(width, height, fmt)
{
	default_brush = new QBrush(Qt::NoBrush);
	fill_brush = new QBrush(Qt::SolidPattern);
	toned_brush_light = new QBrush(Qt::Dense7Pattern);
	toned_brush_mid = new QBrush(Qt::Dense4Pattern);
	toned_brush_deep = new QBrush(Qt::Dense2Pattern);

	_width = width;
	_height = height;
	
}

CSP_DrawItem::~CSP_DrawItem()
{
	delete default_brush;
	delete fill_brush;
	delete toned_brush_light;
	delete toned_brush_mid;
	delete toned_brush_deep;
}

void CSP_DrawItem::drawRectItem(int _x, int _y, int _w, int _h,
								QBrush *brush,
								QColor &_color1, QColor &_color2, Qt::PenStyle style)

{
	QPen pen;
	if(_x < 0) _x = 0;
	if(_y < 0) _y = 0;
	if(_w <= 0) _w = 1;
	if(_h <= 0) _h = 1;
	brush->setColor(_color1);
	painter.setBrush(*brush);
	pen.setColor(_color2);
	pen.setStyle(style);
	painter.setPen(pen);
	painter.drawRect(_x, _y, _w, _h);
	
}

void CSP_DrawItem::drawCircleItem(int _x, int _y, int _r,
								  int _begin, int _end,
								  QBrush *brush,
								  QColor &_color1, QColor &_color2, Qt::PenStyle style)
{
	QPen pen;
	if(_x < 0) _x = 0;
	if(_y < 0) _y = 0;
	if(_r <= 0) _r = 1;
	brush->setColor(_color1);
	painter.setBrush(*brush);
	pen.setColor(_color2);
	pen.setStyle(style);
	painter.setPen(pen);
	painter.drawPie(_x, _y, _r * 2, _r * 2,_begin, _end);
}

void CSP_DrawItem::drawTextItem(int _x, int _y, int _w, int _h, QString &_str,
								QBrush *brush,
								QColor &_color1, QColor & _color2)
{
	if(_x < 0) _x = 0;
	if(_y < 0) _y = 0;
	if(_w <= 0) _w = 1;
	if(_h <= 0) _h = 1;
	brush->setColor(_color1);
	painter.setPen(_color2);
	painter.setBrush(*brush);
	painter.drawText(_x, _y, _w, _h, 0, _str);
	
}

void CSP_DrawItem::drawPolygonItem(QPointF _points[], int members,
								  QBrush *brush,
								   QColor &_color1, QColor &_color2, Qt::PenStyle style)
{
	QPen pen;
	if((_points == NULL) || (members <= 0)) return;
	brush->setColor(_color1);
	painter.setBrush(*brush);
	pen.setColor(_color2);
	pen.setStyle(style);
	painter.setPen(pen);
	painter.drawConvexPolygon(_points, members);
}


void CSP_DrawItem::drawFloppy5Inch(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &TextColor, float text_pt, QString text)
{
	double __width = (double)_width;
	double __height = (double)_height;
	this->fill(BGColor);
	{
		double _nwidth = (__width > __height) ? (__height - 2.0) : (__width - 2.0) ;
		_nwidth = _nwidth * (0.88 * 0.90);
		painter.begin(this);
		int xbase = (int)((__width - _nwidth) / 2.0);
		int ybase = (int)((__height - _nwidth) * 0.5);
		int wbase = (int)_nwidth;
		int hbase = wbase;
		drawRectItem(xbase, ybase, wbase, hbase, fill_brush, FGColor, FGColor);
		{
			int r = (int)((_nwidth * 0.35) / 2.0);
			int x = xbase + wbase / 2 - r;
			int y = ybase + hbase / 2 - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, FGColor2, FGColor2);
		}
		{
			int r = (int)((_nwidth * 0.15) / 2.0);
			int x = xbase + wbase / 2 - r;
			int y = ybase + hbase / 2 - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, BGColor, BGColor);
		}
		{
			double w = _nwidth * 0.15;
			double h = _nwidth * (1.0 - 0.35 - 0.2) / 2.0;
			int x = (xbase + wbase / 2) - (int)(w / 2.0); 
			int y = (ybase + wbase / 2) + (int)(_nwidth * 0.45 / 2.0); 
			drawRectItem(x, y, w, h, fill_brush, FGColor2, FGColor2);
		}
		if(!text.isEmpty()) {
			int x = xbase + (int)(_nwidth * 0.65); 
			int y = ybase + (int)(_nwidth * 0.50);
			int w = (int)(__width * 0.45);
			int h = w;

			drawTextItem(x, y, w, h, text, fill_brush, TextColor, TextColor);
		}
		painter.end();
	}
}
	

void CSP_DrawItem::drawFloppy3_5Inch(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &FGColor3, QColor &TextColor, float text_pt, QString text)
{
	double __width = (double)_width;
	double __height = (double)_height;
	this->fill(BGColor);
	{
		double _nwidth = (__width > __height) ? (__height - 2.0) : (__width - 2.0) ;
		double _nheight;
		_nwidth = _nwidth * 0.88;
		_nheight = _nwidth * 0.88;
		painter.begin(this);
		int xbase = (int)((__width - _nwidth) / 2.0);
		int ybase = (int)((__height - _nheight) / 2.0);
		int wbase = (int)_nwidth;
		int hbase = (int)_nheight;
		{
			QPointF points[5] = {
				QPointF((float)xbase, (float)ybase),
				QPointF((float)xbase + (float)(_nwidth * 0.90), (float)ybase),
				QPointF((float)xbase + (float)_nwidth, (float)ybase + (float)(_nheight * 0.10)),
				QPointF((float)xbase + (float)_nwidth, (float)ybase + (float)_nheight),
				QPointF((float)xbase,  (float)ybase + (float)_nheight)
			};
			drawPolygonItem(points, 5, fill_brush, FGColor, FGColor);
		}
		{
			float xoffset = (float)(_nwidth * 0.25);
			float _w = (float)(_nwidth * (1 - 0.25 * 2.0));
			float _h = (float)(_nheight * 0.45);
			QPointF points[4] = {
				QPointF((float)xbase + xoffset, (float)ybase),
				QPointF((float)xbase + xoffset + _w, (float)ybase), 
				QPointF((float)xbase + xoffset + _w, (float)ybase + _h), 
				QPointF((float)xbase + xoffset, (float)ybase + _h), 
			};
			drawPolygonItem(points, 4, fill_brush, BGColor, BGColor);
			drawPolygonItem(points, 4, fill_brush, FGColor2, FGColor2);
		
			{
				xoffset = xoffset + _w * 0.65;
				float _w2 = _w * 0.25;
				float _h2 = _h * 0.75;
				float yoffset = (float)(_nheight * 0.05); 
				QPointF points2[4] = {
					QPointF((float)xbase + xoffset, (float)ybase + yoffset),
					QPointF((float)xbase + xoffset + _w2, (float)ybase + yoffset), 
					QPointF((float)xbase + xoffset + _w2, (float)ybase + yoffset + _h2), 
					QPointF((float)xbase + xoffset, (float)ybase + yoffset + _h2), 
				};
				drawPolygonItem(points2, 4, fill_brush, FGColor3, FGColor3);
			}
		}
		if(!text.isEmpty()) {
			int x = xbase + + (int)(_nwidth * 0.65); 
			int y = ybase + (int)(_nwidth * 0.55);
			int w = (int)(_nwidth * 0.40);
			int h = w;
			drawTextItem(x, y, w, h, text, fill_brush, TextColor, TextColor);
		}
		painter.end();
	}
}

void CSP_DrawItem::drawCasetteTape(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &FGColor3, QColor &TextColor, float text_pt, QString text)
{
	double __width = (double)_width;
	double __height = (double)_height;
	this->fill(BGColor);
	{
		double _nwidth = (__width > __height) ? (__height - 2.0) : (__width - 2.0) ;
		_nwidth = _nwidth * 0.88;
		double _nheight = _nwidth * 0.7;
		painter.begin(this);
		int xbase = (int)((__width - _nwidth) / 2.0);
		int ybase = (int)((__height - _nheight) / 2.0);
		int wbase = (int)_nwidth;
		int hbase = (int)_nheight;
		{
			int x = xbase;
			int y = ybase;
			int w = wbase;
			int h = hbase;
			drawRectItem(x, y, w, h, fill_brush, BGColor, BGColor);
		}
		{
			int r = (int)((_nwidth * 0.45) / 2.0);
			int x = xbase + (int)(_nwidth * 0.25) - r;
			int y = ybase + (int)(_nheight * 0.5) - r;
			drawCircleItem(x, y, r, 0, 360 * 16,  fill_brush, FGColor, FGColor, Qt::NoPen);
		}
		{
			int r = (int)((_nwidth * 0.10) / 2.0);
			int x = xbase + (int)(_nwidth * 0.25) - r;
			int y = ybase + (int)(_nheight * 0.5) - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, BGColor, BGColor, Qt::NoPen);
		}
		{
			int r = (int)((_nwidth * 0.25) / 2.0);
			int x = xbase + (int)(_nwidth * 0.75) - r;
			int y = ybase + (int)(_nheight * 0.5) - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, FGColor, FGColor, Qt::NoPen);
		}
		{
			int r = (int)((_nwidth * 0.10) / 2.0);
			int x = xbase + (int)(_nwidth * 0.75) - r;
			int y = ybase + (int)(_nheight * 0.5) - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, BGColor, BGColor, Qt::NoPen);
		}
		{
			int x = xbase;
			int y = ybase;
			int w = wbase;
			int h = (int)(_nheight * 0.35);
			drawRectItem(x, y, w, h, fill_brush, FGColor2, FGColor2, Qt::NoPen);
		}
		{
			int x = xbase;
			int y = ybase + (int)(_nheight * 0.65);
			int w = wbase;
			int h = (int)(_nheight * 0.3);
			drawRectItem(x, y, w, h, fill_brush, FGColor2, FGColor2, Qt::NoPen);
		}
		{
			int x = xbase;
			int y = ybase;
			int w = (int)(_nwidth * 0.1);
			int h = hbase;
			drawRectItem(x, y, w, h, fill_brush, FGColor2, FGColor2, Qt::NoPen);
		}
		{
			int w = (int)(_nwidth * 0.1);
			int x = xbase + wbase - w;
			int y = ybase;
			int h = hbase;
			drawRectItem(x, y, w, h, fill_brush, FGColor2, FGColor2, Qt::NoPen);
		}
		if(!text.isEmpty()) {
			int x = 0 + (int)(__width * 0.7); 
			int y = 0 + (int)(__height * 0.5);
			int w = (int)(__width * 0.50);
			int h = w;
			drawTextItem(x, y, w, h, text, fill_brush, TextColor, TextColor);
			fill_brush->setColor(TextColor);
		}
		painter.end();
	}
}

void CSP_DrawItem::drawQuickDisk(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &TextColor, float text_pt, QString text)
{
	double __width = (double)_width;
	double __height = (double)_height;
	this->fill(BGColor);
	{
		double _nwidth = (__width > __height) ? (__height - 2.0) : (__width - 2.0) ;
		_nwidth = _nwidth * (0.88 * 0.90);
		painter.begin(this);
		int xbase = (int)((__width - _nwidth) / 2.0);
		int ybase = (int)((__height - _nwidth) * 0.5);
		int wbase = (int)_nwidth;
		int hbase = wbase;
		drawRectItem(xbase, ybase, wbase, hbase, fill_brush, FGColor2, FGColor2);
		{
			int r = (int)((_nwidth * 0.40) / 2.0);
			int x = xbase + wbase / 2 - r;
			int y = ybase + hbase / 2 - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, FGColor, FGColor);
		}
		{
			double w = _nwidth * 0.15;
			double h = _nwidth * 0.35;
			int x = (xbase + wbase / 2) - (int)(w / 2.0); 
			int y = ybase + wbase / 2 + (int)(_nwidth * 0.20); 
			drawRectItem(x, y, w, h, fill_brush, FGColor, FGColor);
		}
		if(!text.isEmpty()) {
			int x = xbase + (int)(_nwidth * 0.65); 
			int y = ybase + (int)(_nwidth * 0.50);
			int w = (int)(__width * 0.50);
			int h = w;
			drawTextItem(x, y, w, h, text, fill_brush, TextColor, TextColor);
		}
		painter.end();
	}
}

void CSP_DrawItem::drawCompactDisc(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &FGColor3, QColor &LabelColor, QColor &TextColor, float text_pt, QString text)
{
	double __width = (double)_width;
	double __height = (double)_height;
	this->fill(BGColor);
	{
		double _nwidth = (__width > __height) ? (__height - 2.0) : (__width - 2.0) ;
		_nwidth = _nwidth *  0.90;
		painter.begin(this);
		double _nr = _nwidth / 2.0;
		int xbase = (int)(__width / 2.0);
		int ybase = (int)(__height / 2.0);
		int rbase = (int)_nr;
		{
			int r = (int)_nr;
			int x = xbase - rbase;
			int y = ybase - rbase;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, FGColor2, FGColor2, Qt::NoPen);
		}
		{
			int r = (int)(_nr * 0.41 * 1.2);
			int x = xbase - r;
			int y = ybase - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, FGColor3, FGColor3);
		}
		{
			int r = (int)(_nr * 0.35 * 1.2);
			int x = xbase - r;
			int y = ybase - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, BGColor, BGColor);
		}
		{
			int r = (int)(_nr * 0.22 * 1.2);
			int x = xbase - r;
			int y = ybase - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, FGColor2, FGColor2);
		}
		{
			int r = (int)(_nr * 0.15 * 1.2);
			int x = xbase - r;
			int y = ybase - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, BGColor, BGColor);
		}
#if 0		
		{
			int x = (int)(_nwidth * 0.10); 
			int y = (int)(_nwidth * 0.10);
			int w = (int)(__width * 0.40);
			int h = w;
			QString _str;
			_str = QString::fromUtf8("CD");
			drawTextItem(x, y, w, h, _str, fill_brush, LabelColor, LabelColor);
		}
#endif
		if(!text.isEmpty()) {
			int x = (int)(_nwidth * 0.6); 
			int y = (int)(_nwidth * 0.5);
			int w = (int)(__width * 0.40);
			int h = w;

			drawTextItem(x, y, w, h, text, fill_brush, TextColor, TextColor);
		}
		painter.end();
	}
}

void CSP_DrawItem::drawLaserDisc(QColor &BGColor, QColor &FGColor, QColor &FGColor2, QColor &LabelColor, QColor &TextColor, float text_pt, QString text)
{
	double __width = (double)_width;
	double __height = (double)_height;
	this->fill(BGColor);
	{
		double _nwidth = (__width > __height) ? (__height - 2.0) : (__width - 2.0) ;
		_nwidth = _nwidth *  0.95;
		painter.begin(this);
		double _nr = _nwidth / 2.0;
		int xbase = (int)(__width / 2.0);
		int ybase = (int)(__height / 2.0);
		int rbase = (int)_nr;
		{
			int r = (int)_nr;
			int x = xbase - rbase;
			int y = ybase - rbase;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, FGColor2, FGColor2, Qt::NoPen);
		}
		{
			int r = (int)(_nr * 0.30 * 1.2);
			int x = xbase - r;
			int y = ybase - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, FGColor, FGColor, Qt::NoPen);
		}
		{
			int r = (int)(_nr * 0.15 * 1.2);
			int x = xbase - r;
			int y = ybase - r;
			drawCircleItem(x, y, r, 0, 360 * 16, fill_brush, BGColor, BGColor, Qt::NoPen);
		}
#if 0		
		{
			int x = (int)(_nwidth * 0.10); 
			int y = (int)(_nwidth * 0.10);
			int w = (int)(__width * 0.40);
			int h = w;
			QString _str;
			_str = QString::fromUtf8("CD");
			drawTextItem(x, y, w, h, _str, fill_brush, LabelColor, LabelColor);
		}
#endif
		if(!text.isEmpty()) {
			int x = (int)(_nwidth * 0.6); 
			int y = (int)(_nwidth * 0.5);
			int w = (int)(__width * 0.40);
			int h = w;

			drawTextItem(x, y, w, h, text, fill_brush, TextColor, TextColor);
		}
		painter.end();
	}
}

void CSP_DrawItem::clearCanvas(QColor &clColor)
{
	this->fill(clColor);
}
