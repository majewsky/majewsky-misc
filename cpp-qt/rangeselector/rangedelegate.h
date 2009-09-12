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

#ifndef RANGESELECTOR_RANGEDELEGATE_H
#define RANGESELECTOR_RANGEDELEGATE_H

#include "rangeselector.h"

class QRangeSelector;

/**
	* \class QRangeDelegate
	* \since 2.0
	*
	* This helper class for QRangeSelector draws the contents inside the frame, above which the QRangeSelector's sliders move.
	*/
class QRangeDelegate : public QObject
{
	public:
		///Constructs a new RangeDelegate instance.
		QRangeDelegate();

		///Implement this function in a subclass to draw contents inside the frame of the current QRangeSelector instance (with the given \a painter). During this method, the protected getter functions return meaningful values for that QRangeSelector instance.
		virtual void render(QPainter* painter) = 0;
		///\internal
		///Entry point for QRangeSelector. Extracts some properties from the QRangeSelector, then passes control to the render() method, i.e., to the subclass implementation.
		void doRender(QPainter* painter, const QRangeSelector* rs);
	protected:
		///Returns the logical value of the given \a position.
		///\warning Returns valid values only during calls to the render() method.
		int logicalPosition(QRangeSelector::PositionType position) const;
		///Returns the relative value of the given \a position. The relative coordinates are defined by:
		///\code
		///relativePosition(QRangeSelector::Minimum) == 0
		///relativePosition(QRangeSelector::Maximum) == 0
		///\endcode
		///\warning Returns valid values only during calls to the render() method.
		int physicalPosition(QRangeSelector::PositionType position) const;
		///Returns the physical value of the given \a position. The physical coordinates are defined by the widget:
		///\code
		///physicalPosition(QRangeSelector::Minimum) == rect().left()
		///physicalPosition(QRangeSelector::Maximum) == rect().right()
		///\endcode
		///\warning Returns valid values only during calls to the render() method.
		qreal relativePosition(QRangeSelector::PositionType position) const;
		///Returns the rectangle into which the delegate should draw. (The painter is not clipped to this rectangle by default.)
		///\warning Returns valid values only during calls to the render() method.
		QRect rect() const;
	private:
		QRect m_rect;
		QVector<int> m_logicalPositions;
		QVector<qreal> m_relativePositions;
		QVector<int> m_physicalPositions;
};

#endif // RANGESELECTOR_RANGEDELEGATE_H
