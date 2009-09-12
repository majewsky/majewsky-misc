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

#ifndef RANGESELECTOR_RANGESELECTOR_H
#define RANGESELECTOR_RANGESELECTOR_H

#include <QFrame>

class QRangeDelegate;
class QRangeSelectorPrivate;

/**
	* \class QRangeSelector
	* \since 2.0
	*
	* The QRangeSelector is a multi-purpose widget for the selection of ranges and values from a certain range. They are different from sliders and spinboxes in two points:
	* \li They can also be used to select ranges, where one would normally need multiple sliders or spinboxes.
	* \li They can display a gradient in the selection area to graphically indicate the possible values (or show selected ranges).
	* A QRangeSelector is always oriented horizontally.
	*
	* \warning This class differentiates between "range" and "value range". The first one is the range of possible values. From this range, one can select minimum and maximum of the "value range". In other words: The "range" cannot be directly adjusted by the user, but the "value range" can.
	*/
class QRangeSelector : public QFrame
{
	Q_OBJECT
	Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
	Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
	Q_PROPERTY(int maximumValue READ maximumValue WRITE setMaximumValue)
	Q_PROPERTY(int minimumValue READ minimumValue WRITE setMinimumValue)
	Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged USER true)
	Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
	Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
	Q_DISABLE_COPY(QRangeSelector)
	Q_DECLARE_PRIVATE(QRangeSelector)
	public:
		///Specifies what can be selected by the user.
		enum SelectionType
		{
			NoSelection = 0x0,    ///< Nothing can be selected. (The widget only shows the gradient.)
			ValueSelection = 0x1, ///< The user can select only one point. Its value is the "value" property.
			RangeSelection = 0x2, ///< The user can select a range, which is exposed through the "minimumValue" and "maximumValue" properties.
			ThreePointSelection = ValueSelection | RangeSelection ///< A combination of CenterSelection and RangeSelection: The "value" may only be moved in the selected range.
		};

		//NOTE: This is an exact duplicate of QRangeSelectorPrivate::PositionType. Keep this two enums always in sync, or stuff will break!
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

		explicit QRangeSelector(SelectionType type, QWidget* parent = 0);
		virtual ~QRangeSelector();

		SelectionType type() const;
		int pageStep() const;
		int singleStep() const;
		virtual QSize minimumSizeHint() const;
		virtual QSize sizeHint() const;

		int specialPosition(QRangeSelector::PositionType type) const;
		int maximum() const;
		int maximumValue() const;
		int minimum() const;
		int minimumValue() const;
		int value() const;

		void setDelegate(QRangeDelegate* delegate);
		void setSpecialPosition(QRangeSelector::PositionType type, int position);

		void setPageStep(int pageStep);
		void setSingleStep(int singleStep);
		void setMaximum(int maximum);
		void setMaximumValue(int maximumValue);
		void setMinimum(int minimum);
		void setMinimumValue(int minimumValue);
		void setRange(int minimum, int maximum);
		void setValueRange(int minimumValue, int maximumValue);
	public Q_SLOTS:
		void setValue(int value);
	Q_SIGNALS:
		void rangeChanged(int minimum, int maximum);
		void valueRangeChanged(int minimumValue, int maximumValue);
		void valueChanged(int value);
	protected:
		virtual void changeEvent(QEvent* event);
		virtual void paintEvent(QPaintEvent* event);
		virtual void resizeEvent(QResizeEvent* event);

		virtual void focusInEvent(QFocusEvent* event);
		virtual void focusOutEvent(QFocusEvent* event);
		virtual bool focusNextPrevChild(bool next);
		virtual void keyPressEvent(QKeyEvent* event);
		virtual void mousePressEvent(QMouseEvent* event);
		virtual void mouseMoveEvent(QMouseEvent* event);
	private:
		friend class QRangeDelegate;
		QRangeSelectorPrivate* d_ptr;
};

#endif // RANGESELECTOR_RANGESELECTOR_H
