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

#include <climits>
#include <QStandardItemModel>

Utils::ModelListModel::ModelListModel(QObject* parent)
	: QAbstractItemModel(parent)
	, m_metaModel(new QStandardItemModel)
	, m_headerDataSubModel(0)
{
}

Utils::ModelListModel::~ModelListModel()
{
	delete m_metaModel;
	//submodels are automatically deleted because of QObject::setParent
}

void Utils::ModelListModel::addSubModel(const QString& caption, QAbstractListModel* subModel)
{
	if (!m_subModels.contains(subModel)) //avoid memleak in QStandardItem construction
		addSubModelInternal(new QStandardItem(caption), subModel);
}

void Utils::ModelListModel::addSubModel(const QString& caption, QAbstractTableModel* subModel)
{
	if (!m_subModels.contains(subModel)) //avoid memleak in QStandardItem construction
		addSubModelInternal(new QStandardItem(caption), subModel);
}

void Utils::ModelListModel::addSubModel(QStandardItem* metaItem, QAbstractListModel* subModel)
{
	addSubModelInternal(metaItem, subModel);
}

void Utils::ModelListModel::addSubModel(QStandardItem* metaItem, QAbstractTableModel* subModel)
{
	addSubModelInternal(metaItem, subModel);
}

void Utils::ModelListModel::addSubModelInternal(QStandardItem* metaItem, QAbstractItemModel* subModel)
{
	if (m_subModels.contains(subModel))
		return;
	const int newRow = m_subModels.count();
	beginInsertRows(QModelIndex(), newRow, newRow);
	metaItem->setEditable(false);
	m_metaModel->appendRow(metaItem);
	m_subModels << subModel;
	subModel->QObject::setParent(this);
	endInsertRows();
	//connect signals
	connect(subModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex&, int, int)), this, SLOT(handleColumnsAboutToBeInserted(const QModelIndex&, int, int)));
	connect(subModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex&, int, int)), this, SLOT(handleColumnsAboutToBeRemoved(const QModelIndex&, int, int)));
	connect(subModel, SIGNAL(columnsInserted(const QModelIndex&, int, int)), this, SLOT(handleColumnsInserted()));
	connect(subModel, SIGNAL(columnsRemoved(const QModelIndex&, int, int)), this, SLOT(handleColumnsRemoved()));
	connect(subModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(handleDataChanged(const QModelIndex&, const QModelIndex&)));
	connect(subModel, SIGNAL(headerDataChanged(Qt::Orientation, int, int)), this, SLOT(handleHeaderDataChanged(Qt::Orientation, int, int)));
	connect(subModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)), this, SLOT(handleRowsAboutToBeInserted(const QModelIndex&, int, int)));
	connect(subModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), this, SLOT(handleRowsAboutToBeRemoved(const QModelIndex&, int, int)));
	connect(subModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(handleRowsInserted()));
	connect(subModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(handleRowsRemoved()));
	connect(subModel, SIGNAL(destroyed(QObject*)), this, SLOT(handleSubModelDeleted(QObject*)));
}

void Utils::ModelListModel::removeSubModel(QAbstractItemModel* subModel)
{
	//NOTE: This may not call any methods of the model, because this method is called by Utils::ModelListModel::handleSubModelDeleted, which is invoked by the submodel's QObject::destroyed signal.
	const int index = m_subModels.indexOf(subModel);
	if (index == -1)
		return;
	beginRemoveRows(QModelIndex(), index, index);
	m_metaModel->removeRow(index);
	m_subModels.removeAt(index);
	if (subModel->QObject::parent() == this)
		subModel->QObject::setParent(0);
	endRemoveRows();
	disconnect(subModel, 0, this, 0);
	if (m_headerDataSubModel == subModel)
		setHeaderDataSubModel(0);
}

void Utils::ModelListModel::setHeaderDataSubModel(QAbstractItemModel* subModel)
{
	if (!subModel || !m_subModels.contains(subModel)) //except for model == 0, allow only submodels that have been added to this model
		return;
	if (m_headerDataSubModel != subModel)
	{
		m_headerDataSubModel = subModel;
		//announce change
		emit headerDataChanged(Qt::Horizontal, INT_MIN, INT_MAX); //TODO: Is this call valid (with the parameters INT_{MIN,MAX})?
		emit headerDataChanged(Qt::Vertical, INT_MIN, INT_MAX);
	}
}

QModelIndex Utils::ModelListModel::mapFromSource(const Utils::ModelListModel::SubModelIndex& index) const
{
	//validate input
	const QModelIndex& subIndex = index.second;
	QAbstractItemModel* subModel = safeModelCast(index.first);
	if (!subModel)
		return QModelIndex();
	if (!subIndex.isValid())
	{
		//the root item of the metamodel is the root item of the modellistmodel
		if (subModel == m_metaModel)
			return QModelIndex();
		//the root item of a submodel is the respective item in the metamodel
		else
		{
			int modelPos = m_subModels.indexOf(subModel);
			return createIndex(modelPos, 0, subModel);
		}
	}
	else if (subIndex.model() != subModel)
		return QModelIndex();
	//encode the submodel that is responsible for this item into the internal pointer of the model index
	//NOTE: Exactly this is the reason why we cannot embed tree-shaped models into the modellistmodel.
	return createIndex(subIndex.row(), subIndex.column(), subModel);
}

Utils::ModelListModel::SubModelIndex Utils::ModelListModel::mapToSource(const QModelIndex& index) const
{
	QAbstractItemModel* metaModel = m_metaModel; //cast from QStandardItemModel*
	//ensure that the root item of the modellistmodel is provided by the metamodel
	if (!index.isValid())
		return qMakePair(metaModel, QModelIndex());
	//find the source model for this index
	QAbstractItemModel* subModel = safeModelCast(index.internalPointer());
	if (!subModel)
		return qMakePair(metaModel, QModelIndex());
	//create new index for source model
	QModelIndex subIndex = subModel->index(index.row(), index.column());
	return qMakePair(subModel, subIndex);
}

QAbstractItemModel* Utils::ModelListModel::safeModelCast(void* model) const
{
	QAbstractItemModel* modelPtr = reinterpret_cast<QAbstractItemModel*>(model);
	bool knownModel = m_subModels.contains(modelPtr) || modelPtr == m_metaModel;
	return knownModel ? modelPtr : 0;
}

//BEGIN QAbstractItemModel reimplementation
//NOTE on the general implementation: We use mapToSource, and transfer the call to the appropriate submodel. Special cases are only necessary at those places where the submodels are "mounted" into the base tree provided by the meta model.

int Utils::ModelListModel::columnCount(const QModelIndex& parent) const
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(parent);
	if (smi.first == m_metaModel && smi.second.isValid())
	{
		QAbstractItemModel* subModel = m_subModels.value(smi.second.row());
		if (subModel)
			return subModel->columnCount();
	}
	return smi.first->columnCount(smi.second);
}

QVariant Utils::ModelListModel::data(const QModelIndex& index, int role) const
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(index);
	return smi.first->data(smi.second, role);
}

Qt::ItemFlags Utils::ModelListModel::flags(const QModelIndex& index) const
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(index);
	return smi.first->flags(smi.second);
}

QVariant Utils::ModelListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (m_headerDataSubModel)
		return m_headerDataSubModel->headerData(section, orientation, role);
	else
		return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex Utils::ModelListModel::index(int row, int column, const QModelIndex& parent) const
{
	//read parent modelindex
	Utils::ModelListModel::SubModelIndex smi = mapToSource(parent);
	//create SubModelIndex for child
	Utils::ModelListModel::SubModelIndex newSmi;
	if (smi.first == m_metaModel && smi.second.isValid())
	{
		QAbstractItemModel* subModel = m_subModels.value(smi.second.row());
		if (subModel)
		{
			newSmi.first = subModel;
			newSmi.second = subModel->index(row, column);
		}
		else
		{
			newSmi.first = smi.first;
			newSmi.second = smi.first->index(row, column, smi.second);
		}
	}
	else
	{
		newSmi.first = smi.first;
		newSmi.second = smi.first->index(row, column, smi.second);
	}
	//collapse into modelindex
	return mapFromSource(newSmi);
}

bool Utils::ModelListModel::insertColumns(int column, int count, const QModelIndex& parent)
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(parent);
	if (smi.first == m_metaModel && smi.second.parent().isValid())
	{
		QAbstractItemModel* subModel = m_subModels.value(smi.second.row());
		if (subModel)
			return subModel->insertColumns(column, count);
	}
	return smi.first->insertColumns(column, count, smi.second);
}

bool Utils::ModelListModel::insertRows(int row, int count, const QModelIndex& parent)
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(parent);
	if (smi.first == m_metaModel && smi.second.parent().isValid())
	{
		QAbstractItemModel* subModel = m_subModels.value(smi.second.row());
		if (subModel)
			return subModel->insertRows(row, count);
	}
	return smi.first->insertRows(row, count, smi.second);
}

QMap<int, QVariant> Utils::ModelListModel::itemData(const QModelIndex& index) const
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(index);
	return smi.first->itemData(smi.second);
}

QModelIndex Utils::ModelListModel::parent(const QModelIndex& index) const
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(index);
	QModelIndex parent = smi.first->parent(smi.second);
	if (smi.first == m_metaModel || parent.isValid())
		return parent;
	else
	{
		//This item from a submodel has a parent in the metamodel.
		Utils::ModelListModel::SubModelIndex smiParent;
		smiParent.first = m_metaModel;
		smiParent.second = m_metaModel->index(m_subModels.indexOf(smi.first), 0);
		return mapFromSource(smiParent);
	}
}

bool Utils::ModelListModel::removeColumns(int column, int count, const QModelIndex& parent)
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(parent);
	if (smi.first == m_metaModel && smi.second.parent().isValid())
	{
		QAbstractItemModel* subModel = m_subModels.value(smi.second.row());
		if (subModel)
			return subModel->removeColumns(column, count);
	}
	return smi.first->removeColumns(column, count, smi.second);
}

bool Utils::ModelListModel::removeRows(int row, int count, const QModelIndex& parent)
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(parent);
	if (smi.first == m_metaModel && smi.second.parent().isValid())
	{
		QAbstractItemModel* subModel = m_subModels.value(smi.second.row());
		if (subModel)
			return subModel->removeRows(row, count);
	}
	return smi.first->removeRows(row, count, smi.second);
}

int Utils::ModelListModel::rowCount(const QModelIndex& parent) const
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(parent);
	if (smi.first == m_metaModel && smi.second.isValid())
	{
		QAbstractItemModel* subModel = m_subModels.value(smi.second.row());
		if (subModel)
			return subModel->rowCount();
	}
	return smi.first->rowCount(smi.second);
}

bool Utils::ModelListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(index);
	return smi.first->setData(smi.second, value, role);
}

bool Utils::ModelListModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
	if (m_headerDataSubModel)
		return m_headerDataSubModel->setHeaderData(section, orientation, value, role);
	else
		return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

bool Utils::ModelListModel::setItemData(const QModelIndex& index, const QMap<int, QVariant>& roles)
{
	Utils::ModelListModel::SubModelIndex smi = mapToSource(index);
	return smi.first->setItemData(smi.second, roles);
}

void Utils::ModelListModel::revert()
{
	m_metaModel->revert();
	foreach (QAbstractItemModel* subModel, m_subModels)
		subModel->revert();
}

bool Utils::ModelListModel::submit()
{
	bool success = m_metaModel->submit();
	foreach (QAbstractItemModel* subModel, m_subModels)
		success &= subModel->submit();
	return success;
}

//END QAbstractItemModel reimplementation

//BEGIN event propagation for submodels
//WARNING: This stuff is largely untested. Only insertion/removal of rows in submodels is known to work.

void Utils::ModelListModel::handleColumnsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
	QAbstractItemModel* senderModel = safeModelCast(sender());
	beginInsertColumns(mapFromSource(qMakePair(senderModel, parent)), start, end);
}

void Utils::ModelListModel::handleColumnsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	QAbstractItemModel* senderModel = safeModelCast(sender());
	beginRemoveColumns(mapFromSource(qMakePair(senderModel, parent)), start, end);
}

void Utils::ModelListModel::handleColumnsInserted()
{
	endInsertColumns();
	emit layoutChanged();
}

void Utils::ModelListModel::handleColumnsRemoved()
{
	endRemoveColumns();
	emit layoutChanged();
}

void Utils::ModelListModel::handleDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	QAbstractItemModel* senderModel = safeModelCast(sender());
	emit dataChanged(mapFromSource(qMakePair(senderModel, topLeft)), mapFromSource(qMakePair(senderModel, bottomRight)));
}

void Utils::ModelListModel::handleHeaderDataChanged(Qt::Orientation orientation, int first, int last) //NOTE: check sender() == m_headerDataSubModel
{
	if (sender() == m_headerDataSubModel)
		emit headerDataChanged(orientation, first, last);
}

void Utils::ModelListModel::handleRowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
	QAbstractItemModel* senderModel = safeModelCast(sender());
	beginInsertRows(mapFromSource(qMakePair(senderModel, parent)), start, end);
}

void Utils::ModelListModel::handleRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	QAbstractItemModel* senderModel = safeModelCast(sender());
	beginRemoveRows(mapFromSource(qMakePair(senderModel, parent)), start, end);
}

void Utils::ModelListModel::handleRowsInserted()
{
	endInsertRows();
	emit layoutChanged();
}

void Utils::ModelListModel::handleRowsRemoved()
{
	endRemoveRows();
	emit layoutChanged();
}

void Utils::ModelListModel::handleSubModelDeleted(QObject* model)
{
	removeSubModel(reinterpret_cast<QAbstractItemModel*>(model));
}

//END event propagation for submodels
