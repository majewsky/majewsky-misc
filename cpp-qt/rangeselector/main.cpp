/***************************************************************************
 * Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ***************************************************************************/

//NOTE: This file defines a test case which demoes some functions of QRange{Selector,Delegate}. If you choose to use QRange{Selector,Delegate} in your application, you'll obviously not need to import this file, but instantiate QRangeSelector somewhere in your existing code, and create QRangeDelegate subclasses as necessary.

#include "rangedelegate.h"
#include "rangeselector.h"

#include <QApplication>
#include <QLinearGradient>
#include <QLineEdit>
#include <QPainter>
#include <QVBoxLayout>

class TestDelegate3P : public QRangeDelegate
{
	public:
		virtual void render(QPainter* painter)
		{
			QGradientStops stops;
			stops << QGradientStop(relativePosition(QRangeSelector::Minimum), Qt::white);
			stops << QGradientStop(relativePosition(QRangeSelector::MinimumValue), Qt::white);
			stops << QGradientStop(relativePosition(QRangeSelector::Value), Qt::red);
			stops << QGradientStop(relativePosition(QRangeSelector::MaximumValue), Qt::white);
			stops << QGradientStop(relativePosition(QRangeSelector::Maximum), Qt::white);
			QLinearGradient grad(rect().topLeft(), rect().topRight());
			grad.setStops(stops);
			painter->fillRect(rect(), grad);
		}
};

class TestDelegate1P : public QRangeDelegate
{
	public:
		virtual void render(QPainter* painter)
		{
			QLinearGradient grad(rect().topLeft(), rect().topRight());
			grad.setColorAt(0.0, Qt::black);
			grad.setColorAt(1.0, Qt::white);
			painter->fillRect(rect(), grad);
		}
};

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	QRangeSelector* rs1 = new QRangeSelector(QRangeSelector::ThreePointSelection);
	rs1->setRange(0, 1000);
	rs1->setValueRange(250, 900);
	rs1->setValue(500);
	TestDelegate3P delegate1;
	rs1->setDelegate(&delegate1);

	QRangeSelector* rs2 = new QRangeSelector(QRangeSelector::ValueSelection);
	rs2->setRange(0, 255);
	rs2->setValue(128);
	TestDelegate1P delegate2;
	rs2->setDelegate(&delegate2);

	QLineEdit* l1 = new QLineEdit;
	QLineEdit* l2 = new QLineEdit;

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(l1);
	layout->addWidget(rs1);
	layout->addWidget(l2);
	layout->addWidget(rs2);

	QWidget* container = new QWidget;
	container->setLayout(layout);
	container->show();

	return app.exec();
}
