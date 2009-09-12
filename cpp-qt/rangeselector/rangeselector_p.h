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

#ifndef RANGESELECTOR_RANGESELECTOR_P_H
#define RANGESELECTOR_RANGESELECTOR_P_H

#include "rangeselector.h"
#include "rangedelegate.h"

class QRangeSelectorPrivate
{
	Q_DECLARE_PUBLIC(QRangeSelector)
	public:
		//NOTE: This is an exact duplicate of QRangeSelector::PositionType. Keep this two enums always in sync, or stuff will break!
		enum PositionType
		{
			NullPosition = -1, ///<\internal
			Minimum = 0,
			MinimumValue,
			Value,
			MaximumValue,
			Maximum,
			PositionCount
		};

		QRangeSelectorPrivate(QRangeSelector* widget, QRangeSelector::SelectionType type);

		int mapPhysicalToLogicalPosition(int physicalPosition) const;
		void updateRelativePositions();
		void updatePhysicalPositions();

		void findStyleMetrics();
		void doLayout();

		void drawSelector(QPainter* painter, const QRect& contentsRect, int logicalPosition, bool withFocus);
		void drawSelectors(QPainter* painter, const QRect& contentsRect);
	public: //member variables
		QRangeSelector* q_ptr;
		QRangeDelegate* m_delegate;
		//data (the indices of all vectors are specified by the QRangeSelector::PositionType ennumeration)
		int m_pageStep, m_singleStep;
		QRangeSelector::SelectionType m_type;
		QVector<int> m_logicalPositions;
		QVector<qreal> m_relativePositions;
		QVector<int> m_physicalPositions;
		QVector<bool> m_usedSelectors;
		//focus management
		QRangeSelector::PositionType m_focusedSelector; //index of selector that has focus currently (or -1)
		bool m_mouseFocus;
		//geometry
		int m_arrowOffset;
		int m_arrowPadding; //reserved space for arrows (in vertical direction)
		QSize m_sizeHint;
};

#endif // RANGESELECTOR_RANGESELECTOR_P_H
