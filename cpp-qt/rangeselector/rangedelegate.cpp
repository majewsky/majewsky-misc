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

#include "rangedelegate.h"
#include "rangeselector.h"
#include "rangeselector_p.h"

#include <QPainter>

QRangeDelegate::QRangeDelegate()
{
}

int QRangeDelegate::logicalPosition(QRangeSelector::PositionType position) const
{
	return m_logicalPositions[position];
}

qreal QRangeDelegate::relativePosition(QRangeSelector::PositionType position) const
{
	return m_relativePositions[position];
}

QRectF QRangeDelegate::rect() const
{
	return QRectF(0, 0, 1, 1);
}

void QRangeDelegate::doRender(QPainter* painter, const QRangeSelector* rs)
{
	/* Q_D(const QRangeSelector) = */ const QRangeSelectorPrivate* const d = rs->d_func();
	QRect rect = rs->contentsRect();
	m_logicalPositions = d->m_logicalPositions;
	m_relativePositions = d->m_relativePositions;
	//The following loop is a workaround for a bug in QGradient that misrenders gradients when stops are at the same relative location.
	for (int i = 0; i + 1 < QRangeSelector::PositionCount; ++i)
		m_relativePositions[i + 1] = qMax(m_relativePositions[i + 1], m_relativePositions[i] + 0.0001);
	//transform coordinate system of the painter to have the physical contentsRect and QRectF(0, 0, 1, 1) match
	painter->save();
	if (rs->layoutDirection() == Qt::LeftToRight)
	{
		painter->translate(rect.topLeft());
		painter->scale(rect.width(), rect.height());
	}
	else
	{
		painter->translate(rect.topRight());
		painter->scale(-rect.width(), rect.height());
	}
	//call pure virtual function for the rendering
	render(painter);
	painter->restore();
}
