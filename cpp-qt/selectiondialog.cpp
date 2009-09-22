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

#include "selectiondialog.h"

#include <QPointer>

Utils::SelectionDialog::SelectionDialog(QAbstractItemView* view, QWidget* parent, Qt::WFlags flags)
	: KDialog(parent, flags)
	, m_view(view)
{
	setButtons(KDialog::Ok | KDialog::Cancel);
	setMainWidget(view);
	connect(this, SIGNAL(okClicked()), this, SLOT(handleOkClicked()));
}

QModelIndexList Utils::SelectionDialog::resultIndexes() const
{
	return m_resultIndexes;
}

QModelIndexList Utils::SelectionDialog::exec()
{
	QPointer<Utils::SelectionDialog> ptr(this);
	if (KDialog::exec())
	{
		if (ptr) //NOTE: If the app has recieved a quit event while the dialog's event loop was running, "this" would be a wild pointer.
		{
			return m_view->selectionModel()->selectedIndexes();
		}
	}
	return QModelIndexList(); //dialog was aborted, or application has recieved a quit event while dialog was shown
}

void Utils::SelectionDialog::handleOkClicked()
{
	m_resultIndexes = m_view->selectionModel()->selectedIndexes();
}

#include "selectiondialog.moc"
