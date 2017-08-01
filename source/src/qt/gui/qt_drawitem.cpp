
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

void CSP_DrawItem::drawFloppy5Inch(QColor &BGColor, QColor &FGColor, QColor &TextColor, float text_pt, QString text)
{
	double __width = (double)_width;
	double __height = (double)_height;
	this->fill(BGColor);
	{
		double _nwidth = (__width > __height) ? (__height - 2.0) : (__width - 2.0) ;
		_nwidth = _nwidth * 0.88;
		painter.begin(this);
		int xbase = (int)((__width - _nwidth) / 2.0);
		int ybase = (int)((__height - _nwidth) * 0.90);
		int wbase = (int)_nwidth;
		int hbase = wbase;
		{
			int x = xbase;
			int y = ybase;
			int w = wbase;
			int h = hbase;
			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(w <= 0) w = 1;
			if(h <= 0) h = 1;
			toned_brush_mid->setColor(FGColor);
			painter.setBrush(*toned_brush_mid);
			painter.setPen(FGColor);
			painter.drawRect(x, y, w, h);
		}
		{
			int r = (int)((_nwidth * 0.35) / 2.0);
			int x = xbase + wbase / 2 - r;
			int y = ybase + hbase / 2 - r;

			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(r <= 0) r = 1;
			
			fill_brush->setColor(FGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(FGColor);
			painter.drawPie(x, y, r * 2, r * 2, 0, 360 * 16);
		}
		{
			int r = (int)((_nwidth * 0.15) / 2.0);
			int x = xbase + wbase / 2 - r;
			int y = ybase + hbase / 2 - r;

			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(r <= 0) r = 1;

			fill_brush->setColor(BGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(BGColor);
			painter.drawPie(x, y, r * 2, r * 2, 0, 360 * 16);
		}
		{
			double w = _nwidth * 0.15;
			double h = _nwidth * (1.0 - 0.35 - 0.2) / 2.0;
			int x = (xbase + wbase / 2) - (int)(w / 2.0); 
			int y = (ybase + wbase / 2) + (int)(_nwidth * 0.45 / 2.0); 
			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(w <= 0) w = 1;
			if(h <= 0) h = 1;
			fill_brush->setColor(FGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(FGColor);
			painter.drawRect(x, y, w, h);
		}
		if(!text.isEmpty()) {
			int x = xbase + (int)(_nwidth * 0.10); 
			int y = ybase + (int)(_nwidth * 0.10);
			int w = (int)(__width * 0.45);
			int h = w;
			//QPointF _pt = QPointf(text_pt, text_pt);
			fill_brush->setColor(TextColor);
			painter.setPen(TextColor);
			painter.setBrush(*fill_brush);
			painter.drawText(x, y, w, h, 0, text);
		}
		painter.end();
	}
}
	

void CSP_DrawItem::drawFloppy3_5Inch(QColor &BGColor, QColor &FGColor, QColor &TextColor, float text_pt, QString text)
{
	double __width = (double)_width;
	double __height = (double)_height;
	this->fill(BGColor);
	{
		double _nwidth = (__width > __height) ? (__height - 2.0) : (__width - 2.0) ;
		double _nheight;
		_nwidth = _nwidth * 0.90;
		_nheight = _nwidth * 0.90;
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
			toned_brush_mid->setColor(FGColor);
			painter.setBrush(*toned_brush_mid);
			painter.setPen(FGColor);
			painter.drawConvexPolygon(points, 5);
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
			fill_brush->setColor(BGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(BGColor);
			painter.drawConvexPolygon(points, 4);
			toned_brush_light->setColor(FGColor);
			painter.setBrush(*toned_brush_light);
			painter.setPen(BGColor);
			painter.drawConvexPolygon(points, 4);
			
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
				toned_brush_mid->setColor(FGColor);
				painter.setBrush(*toned_brush_mid);
				painter.setPen(FGColor);
				painter.drawConvexPolygon(points2, 4);
			}
		}
		if(!text.isEmpty()) {
			int x = xbase + + (int)(_nwidth * 0.23); 
			int y = ybase + (int)(_nwidth * 0.55);
			int w = (int)(_nwidth * 0.40);
			int h = w;
			//QPointF _pt = QPointf(text_pt, text_pt);
			fill_brush->setColor(TextColor);
			painter.setPen(TextColor);
			painter.setBrush(*fill_brush);
			painter.drawText(x, y, w, h, 0, text);
		}
		painter.end();
	}
}

void CSP_DrawItem::drawCasetteTape(QColor &BGColor, QColor &FGColor, QColor &TextColor, float text_pt, QString text)
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
			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(w <= 0) w = 1;
			if(h <= 0) h = 1;
			fill_brush->setColor(BGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(FGColor);
			painter.drawRect(x, y, w, h);
		}
		{
			int r = (int)((_nwidth * 0.45) / 2.0);
			int x = xbase + (int)(_nwidth * 0.25) - r;
			int y = ybase + (int)(_nheight * 0.5) - r;

			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(r <= 0) r = 1;
			
			fill_brush->setColor(FGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(FGColor);
			painter.drawPie(x, y, r * 2, r * 2, 0, 360 * 16);
		}
		{
			int r = (int)((_nwidth * 0.10) / 2.0);
			int x = xbase + (int)(_nwidth * 0.25) - r;
			int y = ybase + (int)(_nheight * 0.5) - r;

			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(r <= 0) r = 1;
			fill_brush->setColor(BGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(BGColor);
			painter.drawPie(x, y, r * 2, r * 2, 0, 360 * 16);
		}
		{
			int r = (int)((_nwidth * 0.25) / 2.0);
			int x = xbase + (int)(_nwidth * 0.75) - r;
			int y = ybase + (int)(_nheight * 0.5) - r;
			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(r <= 0) r = 1;
			
			fill_brush->setColor(FGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(FGColor);
			painter.drawPie(x, y, r * 2, r * 2, 0, 360 * 16);

		}
		{
			int r = (int)((_nwidth * 0.10) / 2.0);
			int x = xbase + (int)(_nwidth * 0.75) - r;
			int y = ybase + (int)(_nheight * 0.5) - r;
			fill_brush->setColor(BGColor);
			painter.setBrush(*fill_brush);
			painter.setPen(BGColor);
			painter.drawPie(x, y, r * 2, r * 2, 0, 360 * 16);

		}
		{
			int x = xbase;
			int y = ybase;
			int w = wbase;
			int h = (int)(_nheight * 0.35);
			QPen pen;
			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(w <= 0) w = 1;
			if(h <= 0) h = 1;
			toned_brush_mid->setColor(FGColor);
			painter.setBrush(*toned_brush_mid);
			pen.setColor(FGColor);
			pen.setStyle(Qt::NoPen);
			painter.setPen(pen);
			painter.drawRect(x, y, w, h);
		}
		{
			int x = xbase;
			int y = ybase + (int)(_nheight * 0.65);
			int w = wbase;
			int h = (int)(_nheight * 0.3);
			QPen pen;
			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(w <= 0) w = 1;
			if(h <= 0) h = 1;
			toned_brush_mid->setColor(FGColor);
			painter.setBrush(*toned_brush_mid);
			pen.setColor(FGColor);
			pen.setStyle(Qt::NoPen);
			painter.setPen(pen);
			painter.drawRect(x, y, w, h);
		}
		{
			int x = xbase;
			int y = ybase;
			int w = (int)(_nwidth * 0.2);
			int h = hbase;
			QPen pen;
			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(w <= 0) w = 1;
			if(h <= 0) h = 1;
			toned_brush_mid->setColor(FGColor);
			painter.setBrush(*toned_brush_mid);
			pen.setColor(FGColor);
			pen.setStyle(Qt::NoPen);
			painter.setPen(pen);
			painter.drawRect(x, y, w, h);
		}
		{
			int w = (int)(_nwidth * 0.2);
			int x = xbase + wbase - w;
			int y = ybase;
			int h = hbase;
			QPen pen;
			if(x < 0) x = 0;
			if(y < 0) y = 0;
			if(w <= 0) w = 1;
			if(h <= 0) h = 1;
			toned_brush_mid->setColor(FGColor);
			painter.setBrush(*toned_brush_mid);
			pen.setColor(FGColor);
			pen.setStyle(Qt::NoPen);
			painter.setPen(pen);
			painter.drawRect(x, y, w, h);
		}
		if(!text.isEmpty()) {
			int x = 0 + (int)(_nwidth * 0.1); 
			int y = 0 + (int)(_nheight * 0.1);
			int w = (int)(__width * 0.45);
			int h = w;
			//QPointF _pt = QPointf(text_pt, text_pt);
			fill_brush->setColor(TextColor);
			painter.setPen(TextColor);
			painter.setBrush(*fill_brush);
			painter.drawText(x, y, w, h, 0, text);
		}
		painter.end();
	}
}
