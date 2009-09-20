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

#include "rangeselector.h"
#include "rangeselector_p.h"

#include <QApplication>
#include <QEvent>
#include <QLinearGradient>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

//TODO: Finish API documentation. Consistent naming in API (regarding specialPosition).
//TODO: Handle wheel events?

QLineEdit* someLineEdit = 0;

//BEGIN QRangeSelectorPrivate

QRangeSelectorPrivate::QRangeSelectorPrivate(QRangeSelector* widget, QRangeSelector::SelectionType type)
	: q_ptr(widget)
	, m_delegate(0)
	, m_pageStep(10)
	, m_singleStep(1)
	, m_type(type)
	, m_logicalPositions(PositionCount)
	, m_relativePositions(PositionCount)
	, m_physicalPositions(PositionCount)
	, m_usedSelectors(PositionCount, false)
	, m_focusedSelector(QRangeSelector::NullPosition)
	, m_mouseFocus(false)
	, m_arrowOffsetTipSide(0)
	, m_arrowOffsetBaseSide(0)
	, m_arrowPadding(0)
{
	m_usedSelectors[Value] = type & QRangeSelector::ValueSelection;
	m_usedSelectors[MinimumValue] = m_usedSelectors[MaximumValue] = type & QRangeSelector::RangeSelection;
	findStyleMetrics();
	//provide sane default values for all points
	m_logicalPositions[Minimum] = m_logicalPositions[MinimumValue] = 0;
	m_logicalPositions[Maximum] = m_logicalPositions[MaximumValue] = 100;
	m_logicalPositions[Value] = 50;
	updateRelativePositions();
	updatePhysicalPositions();
}

int QRangeSelectorPrivate::mapPhysicalToLogicalPosition(int physicalPosition) const
{
	Q_Q(const QRangeSelector);
	//gather some values
	const QRect rect = q->contentsRect();
	const int rangeMinimum = m_logicalPositions[Minimum];
	const int rangeMaximum = m_logicalPositions[Maximum];
	const qreal rangeLength = rangeMaximum - rangeMinimum;
	//map physical to logical coordinates
	qreal relativePosition;
	if (q->layoutDirection() == Qt::LeftToRight)
		relativePosition = qreal(physicalPosition - rect.left()) / rect.width();
	else
		relativePosition = qreal(rect.right() - physicalPosition) / rect.width();
	return qRound(relativePosition * rangeLength) + rangeMinimum;
}

void QRangeSelectorPrivate::updateRelativePositions()
{
	const int rangeMinimum = m_logicalPositions[Minimum];
	const int rangeMaximum = m_logicalPositions[Maximum];
	const qreal rangeLength = rangeMaximum - rangeMinimum;
	for (int i = 0; i < PositionCount; ++i)
		m_relativePositions[i] = qreal(m_logicalPositions[i] - rangeMinimum) / rangeLength;
}

void QRangeSelectorPrivate::updatePhysicalPositions()
{
	Q_Q(QRangeSelector);
	const QRect rect = q->contentsRect();
	bool isLTR = q->layoutDirection() == Qt::LeftToRight;
	for (int i = 0; i < PositionCount; ++i)
		if (isLTR)
			m_physicalPositions[i] = qRound(m_relativePositions[i] * rect.width()) + rect.left();
		else
			m_physicalPositions[i] = rect.right() - qRound(m_relativePositions[i] * rect.width());
}

void QRangeSelectorPrivate::findStyleMetrics()
{
	//Problem: The upper end of the PE_IndicatorSpinUp arrows is not necessarily at the top of opt.rect. The following code finds the upper end of the PE_IndicatorSpinUp arrow, to correctly align it with the frame.
	//Note: We have to use the size 16x16, because some styles (e.g. Oxygen) hard-code this size.
	Q_Q(QRangeSelector);
	QStyleOption opt;
	opt.initFrom(q);
	opt.rect = QRect(0, 0, 16, 16);
	//paint arrow
	QImage arrowImage(16, 16, QImage::Format_ARGB32_Premultiplied);
	arrowImage.fill(Qt::transparent);
	QPainter arrowPainter(&arrowImage);
	q->style()->drawPrimitive(QStyle::PE_IndicatorSpinUp, &opt, &arrowPainter);
	arrowPainter.end();
	//look for the upper end of the arrow along the middle axis of the image (i.e., x = 8)
	m_arrowOffsetTipSide = 0;
	while (QColor::fromRgba(arrowImage.pixel(8, m_arrowOffsetTipSide)).alpha() != 255)
		++m_arrowOffsetTipSide;
	m_arrowOffsetBaseSide = 15;
	while (QColor::fromRgba(arrowImage.pixel(8, m_arrowOffsetBaseSide)).alpha() != 255)
		--m_arrowOffsetBaseSide;
	m_arrowPadding = m_arrowOffsetBaseSide - m_arrowOffsetTipSide - q->lineWidth() - q->midLineWidth();
	//set size hint
	if (!someLineEdit)
	{
		someLineEdit = new QLineEdit;
		someLineEdit->QObject::setParent(qApp);
	}
	m_sizeHint = someLineEdit->sizeHint() + QSize(0, 2 * m_arrowPadding);
}

void QRangeSelectorPrivate::doLayout()
{
	Q_Q(QRangeSelector);
	QRect frameRect = q->rect();
	frameRect.adjust(0, m_arrowPadding, 0, -m_arrowPadding);
	q->setFrameRect(frameRect);
	updatePhysicalPositions();
}

void QRangeSelectorPrivate::drawSelector(QPainter* painter, const QRect& contentsRect, int position, bool withFocus)
{
	Q_Q(QRangeSelector);
	//draw line
	painter->save();
	QPen pen = painter->pen();
	pen.setWidth(q->lineWidth() + q->midLineWidth());
	painter->setPen(pen);
	painter->drawLine(position, contentsRect.top(), position, contentsRect.bottom());
	painter->restore();
	if (withFocus)
	{
		//draw arrow at the bottom
		QStyleOption opt;
		opt.initFrom(q);
		opt.rect = QRect(position - 9, contentsRect.bottom() - m_arrowOffsetTipSide, 16, 16);
		q->style()->drawPrimitive(QStyle::PE_IndicatorSpinUp, &opt, painter, q);
		//draw arrow at the top
		opt.rect.moveBottom(contentsRect.top() + m_arrowOffsetTipSide);
		q->style()->drawPrimitive(QStyle::PE_IndicatorSpinDown, &opt, painter, q);
	}
}

void QRangeSelectorPrivate::drawSelectors(QPainter* painter, const QRect& contentsRect)
{
	for (int i = 0; i < PositionCount; ++i)
		if (m_usedSelectors[i])
			drawSelector(painter, contentsRect, m_physicalPositions[i], m_focusedSelector == i);
}

//END QRangeSelectorPrivate

//BEGIN QRangeSelector

QRangeSelector::QRangeSelector(SelectionType type, QWidget* parent)
	: QFrame(parent)
	, d_ptr(new QRangeSelectorPrivate(this, type))
{
	//init frame
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	setLineWidth(2);
	//behavior
	if (type == NoSelection)
		setFocusPolicy(Qt::NoFocus);
	else
		setFocusPolicy(Qt::TabFocus); //The widget may also gain focus through mouse interaction, but this will be done in our implementation of the mouse events.
	setSizePolicy(QSizePolicy(
		/* horizontal policy */ QSizePolicy::Expanding,
		/*   vertical policy */ QSizePolicy::MinimumExpanding,
		/*   behaves like... */ QSizePolicy::Slider
	));
	setMinimumSize(minimumSizeHint());
}

QRangeSelector::~QRangeSelector()
{
	delete d_ptr;
}

QRangeSelector::SelectionType QRangeSelector::type() const
{
	Q_D(const QRangeSelector);
	return d->m_type;
}

int QRangeSelector::pageStep() const
{
	Q_D(const QRangeSelector);
	return d->m_pageStep;
}

int QRangeSelector::singleStep() const
{
	Q_D(const QRangeSelector);
	return d->m_singleStep;
}

void QRangeSelector::setPageStep(int pageStep)
{
	Q_D(QRangeSelector);
	d->m_pageStep = pageStep;
}

void QRangeSelector::setSingleStep(int singleStep)
{
	Q_D(QRangeSelector);
	d->m_singleStep = singleStep;
}

int QRangeSelector::specialPosition(QRangeSelector::PositionType type) const
{
	Q_D(const QRangeSelector);
	return d->m_logicalPositions[type];
}

int QRangeSelector::maximum() const
{
	Q_D(const QRangeSelector);
	return d->m_logicalPositions[Maximum];
}

int QRangeSelector::maximumValue() const
{
	Q_D(const QRangeSelector);
	return d->m_logicalPositions[MaximumValue];
}

int QRangeSelector::minimum() const
{
	Q_D(const QRangeSelector);
	return d->m_logicalPositions[Minimum];
}

int QRangeSelector::minimumValue() const
{
	Q_D(const QRangeSelector);
	return d->m_logicalPositions[MinimumValue];
}

int QRangeSelector::value() const
{
	Q_D(const QRangeSelector);
	return d->m_logicalPositions[Value];
}

void QRangeSelector::setSpecialPosition(QRangeSelector::PositionType type, int position)
{
	Q_D(QRangeSelector);
	QVector<QRangeSelector::PositionType> changedPositions;
	//normalize input
	if (type == Minimum || type == MinimumValue)
	{
		//NOTE: 2 * Value - type = the counter part of this minimum position
		if (position > d->m_logicalPositions[2 * Value - type])
			position = d->m_logicalPositions[2 * Value - type];
	}
	if (type == Maximum || type == MaximumValue)
	{
		if (position < d->m_logicalPositions[2 * Value - type])
			position = d->m_logicalPositions[2 * Value - type];
	}
	if (type == Value || type == MinimumValue)
	{
		const int lowerBound = d->m_logicalPositions[type - 1];
		if (position < lowerBound)
			position = lowerBound;
	}
	if (type == Value || type == MaximumValue)
	{
		const int upperBound = d->m_logicalPositions[type + 1];
		if (position > upperBound)
			position = upperBound;
	}
	//exit if nothing changes
	if (d->m_logicalPositions[type] == position)
		return;
	//set value
	d->m_logicalPositions[type] = position;
	changedPositions << type;
	//keep value range in sync with range if RangeSelection is disabled
	if (!(d->m_type & RangeSelection))
	{
		if (changedPositions.contains(Minimum))
		{
			d->m_logicalPositions[MinimumValue] = d->m_logicalPositions[Minimum];
			changedPositions << MinimumValue;
		}
		if (changedPositions.contains(Maximum))
		{
			d->m_logicalPositions[MaximumValue] = d->m_logicalPositions[Maximum];
			changedPositions << MaximumValue;
		}
	}
	//look for other values that have to be adjusted because of this change
	if (changedPositions.contains(Minimum))
	{
		if (d->m_logicalPositions[MinimumValue] < d->m_logicalPositions[Minimum])
		{
			d->m_logicalPositions[MinimumValue] = d->m_logicalPositions[Minimum];
			changedPositions << MinimumValue;
		}
	}
	if (changedPositions.contains(Maximum))
	{
		if (d->m_logicalPositions[MaximumValue] > d->m_logicalPositions[Maximum])
		{
			d->m_logicalPositions[MaximumValue] = d->m_logicalPositions[Maximum];
			changedPositions << MaximumValue;
		}
	}
	if (changedPositions.contains(MinimumValue))
	{
		if (d->m_logicalPositions[Value] < d->m_logicalPositions[MinimumValue])
		{
			d->m_logicalPositions[Value] = d->m_logicalPositions[MinimumValue];
			changedPositions << Value;
		}
	}
	if (changedPositions.contains(MaximumValue))
	{
		if (d->m_logicalPositions[Value] > d->m_logicalPositions[MaximumValue])
		{
			d->m_logicalPositions[Value] = d->m_logicalPositions[MaximumValue];
			changedPositions << Value;
		}
	}
	//emit signals for changes
	if (changedPositions.contains(Minimum) || changedPositions.contains(Maximum))
		emit rangeChanged(d->m_logicalPositions[Minimum], d->m_logicalPositions[Maximum]);
	if (changedPositions.contains(MinimumValue) || changedPositions.contains(MaximumValue))
		emit valueRangeChanged(d->m_logicalPositions[MinimumValue], d->m_logicalPositions[MaximumValue]);
	if (changedPositions.contains(Value))
		emit valueChanged(d->m_logicalPositions[Value]);
	//updates
	d->updateRelativePositions();
	d->updatePhysicalPositions();
	update();
}

void QRangeSelector::setMinimum(int minimum)
{
	setSpecialPosition(Minimum, minimum);
}

void QRangeSelector::setMaximum(int maximum)
{
	setSpecialPosition(Maximum, maximum);
}

void QRangeSelector::setRange(int minimum, int maximum)
{
	setSpecialPosition(Minimum, minimum);
	setSpecialPosition(Maximum, maximum);
	setSpecialPosition(Minimum, minimum); //We set the minimum for a second time because it might have been restricted by the old maximum.
}

void QRangeSelector::setMinimumValue(int minimumValue)
{
	setSpecialPosition(MinimumValue, minimumValue);
}

void QRangeSelector::setMaximumValue(int maximumValue)
{
	setSpecialPosition(MaximumValue, maximumValue);
}

void QRangeSelector::setValueRange(int minimumValue, int maximumValue)
{
	setSpecialPosition(MinimumValue, minimumValue);
	setSpecialPosition(MaximumValue, maximumValue);
	setSpecialPosition(MinimumValue, minimumValue); //We set the minimum value for a second time because it might have been restricted by the old maximum.
}

void QRangeSelector::setValue(int value)
{
	setSpecialPosition(Value, value);
}

void QRangeSelector::setDelegate(QRangeDelegate* delegate)
{
	Q_D(QRangeSelector);
	d->m_delegate = delegate;
	update();
}

QSize QRangeSelector::minimumSizeHint() const
{
	Q_D(const QRangeSelector);
	return d->m_sizeHint;
}

QSize QRangeSelector::sizeHint() const
{
	Q_D(const QRangeSelector);
	return d->m_sizeHint;
}

void QRangeSelector::changeEvent(QEvent* event)
{
	Q_D(QRangeSelector);
	if (event->type() == QEvent::StyleChange)
	{
		d->findStyleMetrics();
		d->doLayout();
		setMinimumSize(minimumSizeHint());
	}
	QFrame::changeEvent(event);
}

void QRangeSelector::paintEvent(QPaintEvent* /*event*/)
{
	Q_D(QRangeSelector);
	//draw frame contents
	QPainter p(this);
	if (d->m_delegate)
		d->m_delegate->doRender(&p, this);
	//draw frame and interface
	QFrame::drawFrame(&p);
	d->drawSelectors(&p, contentsRect());
}

void QRangeSelector::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)
	Q_D(QRangeSelector);
	d->doLayout();
}

void QRangeSelector::focusInEvent(QFocusEvent* event)
{
	Q_D(QRangeSelector);
	//if triggered by keyboard (i.e. the Tab key), we have to activate some selector
	if (event->reason() == Qt::TabFocusReason)
	{
		d->m_focusedSelector = NullPosition; //The next call will set focus on the first child (i.e. the first selector).
		focusNextPrevChild(true);
	}
	else if (event->reason() == Qt::BacktabFocusReason)
	{
		d->m_focusedSelector = PositionCount;
		focusNextPrevChild(false);
	}
	//forward event to base class
	QFrame::focusInEvent(event);
	event->accept();
}

bool QRangeSelector::focusNextPrevChild(bool next)
{
	Q_D(QRangeSelector);
	if (next)
	{
		//find next selector
		for (int i = d->m_focusedSelector + 1; i < PositionCount; ++i)
			if (d->m_usedSelectors[i])
			{
				d->m_focusedSelector = (PositionType) i;
				update(); //TODO: optimize painting area
				return true;
			}
	}
	else
	{
		//find previous selector
		for (int i = d->m_focusedSelector - 1; i >= 0; --i)
			if (d->m_usedSelectors[i])
			{
				d->m_focusedSelector = (PositionType) i;
				update(); //TODO: optimize painting area
				return true;
			}
	}
	//no selector found
	return false;
}

void QRangeSelector::focusOutEvent(QFocusEvent* event)
{
	Q_D(QRangeSelector);
	d->m_focusedSelector = NullPosition;
	//forward event to base class
	QFrame::focusOutEvent(event);
	event->accept();
}

void QRangeSelector::keyPressEvent(QKeyEvent* event)
{
	Q_D(QRangeSelector);
	if (d->m_focusedSelector == NullPosition)
	{
		QFrame::keyPressEvent(event);
		return;
	}
	int selectorPosition = d->m_logicalPositions[d->m_focusedSelector];
	int layoutDirectionSign = (layoutDirection() == Qt::LeftToRight) ? 1 : -1;
	switch (event->key())
	{
		case Qt::Key_Left:
			//minus single step
			selectorPosition -= layoutDirectionSign * d->m_singleStep;
			break;
		case Qt::Key_Right:
			//plus single step
			selectorPosition += layoutDirectionSign * d->m_singleStep;
			break;
		case Qt::Key_PageDown:
			//minus page step
			selectorPosition -= layoutDirectionSign * d->m_pageStep;
			break;
		case Qt::Key_PageUp:
			//plus page step
			selectorPosition += layoutDirectionSign * d->m_pageStep;
			break;
		case Qt::Key_Home:
			//move to minimum
			selectorPosition = d->m_logicalPositions[Minimum] - 10; //will be normalized in setter automatically
			break;
		case Qt::Key_End:
			//move to maximum
			selectorPosition = d->m_logicalPositions[Maximum] + 10; //will be normalized in setter automatically
			break;
		default:
			//event was not handled by this implementation
			QFrame::keyPressEvent(event);
			return;
	}
	setSpecialPosition(d->m_focusedSelector, selectorPosition);
	event->accept();
}

void QRangeSelector::mousePressEvent(QMouseEvent* event)
{
	Q_D(QRangeSelector);
	if (event->button() != Qt::LeftButton)
	{
		QFrame::mousePressEvent(event);
		return;
	}
	//Note: QApplication::startDragDistance is a sensible default for the extent of the mouse interaction range around the selector positions.
	const int maxPhysicalDistance = qMax(QApplication::startDragDistance(), 3);
	const int physicalPosition = event->pos().x();
	const int logicalPosition = d->mapPhysicalToLogicalPosition(physicalPosition);
	//find selectors within the mouse range
	d->m_mouseFocus = false; //reset state
	for (int i = 0; i < PositionCount; ++i)
		if (d->m_usedSelectors[i])
			if (qAbs(physicalPosition - d->m_physicalPositions[i]) <= maxPhysicalDistance)
			{
				d->m_focusedSelector = (PositionType) i;
				setFocus(Qt::MouseFocusReason);
				d->m_mouseFocus = true;
				setSpecialPosition(d->m_focusedSelector, logicalPosition);
				return;
			}
	//event was not handled by this implementation
	event->ignore();
}

void QRangeSelector::mouseMoveEvent(QMouseEvent* event)
{
	Q_D(QRangeSelector);
	if (event->buttons() & Qt::LeftButton && d->m_mouseFocus)
	{
		const int physicalPosition = event->pos().x();
		const int logicalPosition = d->mapPhysicalToLogicalPosition(physicalPosition);
		setSpecialPosition(d->m_focusedSelector, logicalPosition);
		event->accept();
	}
	else
		QFrame::mouseMoveEvent(event);
}

//END QRangeSelector
