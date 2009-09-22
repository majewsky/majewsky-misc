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

#ifndef UTILS_SELECTIONDIALOG_H
#define UTILS_SELECTIONDIALOG_H

#include <QAbstractItemView>
#include <KDialog>

namespace Utils
{
	/**
	 * \class SelectionDialog
	 * This is a dialog that allows the user to select something from some QAbstractItemView. After the QDialog::exec() call, the selected model indexes can be obtained from the resultIndexes() method.
	 * \note The dialog takes ownership of the QAbstractItemView instance you give to it.
	 * This is an example on how to use SelectionDialog as a modal dialog:
\code
QListView* view = new QListView;
... //custom view initialization
QPointer<Utils::SelectionDialog> dialog(new Utils::SelectionDialog(view));
... //custom dialog initialization
foreach (const QModelIndex& index, dialog->exec())
{
	//do something with that index
}
delete dialog;
\endcode
	 */
	class SelectionDialog : public KDialog
	{
		Q_OBJECT
		public:
			explicit SelectionDialog(QAbstractItemView* view, QWidget* parent = 0, Qt::WFlags flags = 0);

			QModelIndexList resultIndexes() const;
			///A convenience method for modal dialogs.
			///\returns an empty list if "Cancel" was clicked, or a list of all selected items if "OK" was clicked
			QModelIndexList exec();
		private Q_SLOTS:
			void handleOkClicked();
		private:
			QModelIndexList m_resultIndexes;
			QAbstractItemView* m_view;
	};
}

#endif // UTILS_SELECTIONDIALOG_H
