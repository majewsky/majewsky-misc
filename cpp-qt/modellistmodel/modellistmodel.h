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

#ifndef UTILS_MODELLISTMODEL_H
#define UTILS_MODELLISTMODEL_H

#include <QAbstractItemModel>
class QStandardItem;
class QStandardItemModel;

namespace Utils
{
	/**
	 * \class Utils::ModelListModel
	 *
	 * This tree-shaped proxy model contains multiple submodels. On its root level, it lists the available submodels, and the data of these models is displayed below these items ("below" in a hierarchical meaning, of course the submodel data is not on the root level). Only flat submodels are allowed (i.e., subclasses of QAbstractListModel and QAbstractTableModel).
	 *
	 * \warning This implementation does not honor all possible properties of the submodels. Most notably, the following virtual methods are not reimplemented in this ModelListModel:
	 * \li incremental population: QAbstractItemModel::canFetchMore, QAbstractItemModel::fetchMore
	 * \li drag/drop and MIME data: QAbstractItemModel::dropMimeData, QAbstractItemModel::mimeData, QAbstractItemModel::mimeTypes, QAbstractItemModel::supportedDropActions
	 * \li submodel layout changes: QAbstractItemModel::layoutAboutToBeChanged, QAbstractItemModel::layoutChanged
	 * \li submodel resetting: QAbstractItemModel::modelAboutToBeReset, QAbstractItemModel::modelReset
	 */
	class ModelListModel : public QAbstractItemModel
	{
		Q_OBJECT
		public:
			ModelListModel(QObject* parent = 0);
			virtual ~ModelListModel();

			void addSubModel(QStandardItem* metaItem, QAbstractListModel* subModel); //DOCNOTE: takes ownership
			void addSubModel(QStandardItem* metaItem, QAbstractTableModel* subModel); //DOCNOTE: overload
			void addSubModel(const QString& caption, QAbstractListModel* subModel); //DOCNOTE: overload
			void addSubModel(const QString& caption, QAbstractTableModel* subModel); //DOCNOTE: overload
			void removeSubModel(QAbstractItemModel* subModel); //DOCNOTE: releases ownership
			void setHeaderDataSubModel(QAbstractItemModel* subModel);

			//QAbstractItemModel reimplementation
			virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
			virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
			virtual Qt::ItemFlags flags(const QModelIndex& index) const;
			virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
			virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
			virtual bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex());
			virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
			virtual QMap<int, QVariant> itemData(const QModelIndex& index) const;
			virtual QModelIndex parent(const QModelIndex& index) const;
			virtual bool removeColumns(int column, int count, const QModelIndex& parent = QModelIndex());
			virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
			virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
			virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
			virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole);
			virtual bool setItemData(const QModelIndex& index, const QMap<int, QVariant>& roles);
		public Q_SLOTS:
			virtual void revert();
			virtual bool submit();
		private Q_SLOTS: //slots for signal forwarding from submodels
			void handleColumnsAboutToBeInserted(const QModelIndex& parent, int start, int end);
			void handleColumnsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
			void handleColumnsInserted();
			void handleColumnsRemoved();
			void handleDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
			void handleHeaderDataChanged(Qt::Orientation orientation, int first, int last);
			void handleRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
			void handleRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
			void handleRowsInserted();
			void handleRowsRemoved();
			void handleSubModelDeleted(QObject* model);
		private:
			typedef QPair<QAbstractItemModel*, QModelIndex> SubModelIndex;

			void addSubModelInternal(QStandardItem* metaItem, QAbstractItemModel* model);
			SubModelIndex mapToSource(const QModelIndex& index) const;
			QModelIndex mapFromSource(const SubModelIndex& index) const;
			QAbstractItemModel* safeModelCast(void* model) const;

			QStandardItemModel* m_metaModel;
			QList<QAbstractItemModel*> m_subModels;
			QAbstractItemModel* m_headerDataSubModel;
	};
}

#endif // UTILS_MODELLISTMODEL_H
