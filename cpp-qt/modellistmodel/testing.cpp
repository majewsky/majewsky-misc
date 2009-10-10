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

#include "modellistmodel.h"
#include "testing.h"

#if 0
MySubModel::MySubModel(const QString& name)
{
	QStringList myList;
	for (int i = 0; i < 5; ++i)
		myList << QString("%1 item %2").arg(name).arg(i);
	setStringList(myList);
}
#endif

MySubView::MySubView(const QString& name)
	: m_view(new QListView)
	, m_model(new QStringListModel(this))
	, m_name(name)
{
	m_view->setModel(m_model);
	//action buttons
	QPushButton* addButton = new QPushButton("Add item");
	connect(addButton, SIGNAL(pressed()), this, SLOT(addItem()));
	QPushButton* delButton = new QPushButton("Remove item");
	connect(delButton, SIGNAL(pressed()), this, SLOT(removeItem()));
	//construct layout
	QGridLayout* l = new QGridLayout;
	l->setMargin(0);
	l->addWidget(m_view, 0, 0, 1, 2);
	l->addWidget(addButton, 1, 0);
	l->addWidget(delButton, 1, 1);
	setLayout(l);
}

void MySubView::addItem()
{
	int pos = m_model->rowCount();
	const QString caption = QString("%1 item %2").arg(m_name).arg(pos + 1);
	m_model->insertRow(pos);
	m_model->setData(m_model->index(pos), caption);
}

void MySubView::removeItem()
{
	int pos = m_model->rowCount() - 1;
	if (pos >= 0)
		m_model->removeRow(pos);
}

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	QGroupBox* subGroup = new QGroupBox("Single models");
	QVBoxLayout* subLayout = new QVBoxLayout;
	subGroup->setLayout(subLayout);
	QGroupBox* mainGroup = new QGroupBox("Main model");
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainGroup->setLayout(mainLayout);

	MySubView* v1 = new MySubView("Foo");
	subLayout->addWidget(v1);
	MySubView* v2 = new MySubView("Bar");
	subLayout->addWidget(v2);
	MySubView* v3 = new MySubView("Baz");
	subLayout->addWidget(v3);

	Utils::ModelListModel* mainModel = new Utils::ModelListModel;
	mainModel->addSubModel("Foo model", v1->model());
	mainModel->addSubModel("Bar model", v2->model());
	mainModel->addSubModel("Baz model", v3->model());

	QTreeView* mainView = new QTreeView;
	mainView->setModel(mainModel);
	mainView->expandAll();
	mainLayout->addWidget(mainView);

	QWidget container;
	QHBoxLayout* containerLayout = new QHBoxLayout;
	container.setLayout(containerLayout);
	containerLayout->addWidget(subGroup);
	containerLayout->addWidget(mainGroup);
	containerLayout->setMargin(0);

	container.show();
	return app.exec();
}
