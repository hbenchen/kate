/* This file is part of the KDE libraries
   Copyright (C) 2005-2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KATECOMPLETIONMODEL_H
#define KATECOMPLETIONMODEL_H

#include <QAbstractProxyModel>
#include <QPair>
#include <QMutableListIterator>
#include <QHash>

#include <ktexteditor/codecompletion2.h>

class KateCompletionWidget;
class KateView;

/**
 * This class has the responsibility for filtering, sorting, and manipulating
 * code completion data provided by a CodeCompletionModel.
 *
 * @author Hamish Rodda <rodda@kde.org>
 */
class KateCompletionModel : public QAbstractProxyModel
{
  Q_OBJECT

  public:
    KateCompletionModel(KateCompletionWidget* parent = 0L);

    KTextEditor::CodeCompletionModel* completionModel() const;

    KateView* view() const;

    virtual void setSourceModel( QAbstractItemModel* sourceModel );

    const QString& currentCompletion() const;
    void setCurrentCompletion(const QString& completion);

    Qt::CaseSensitivity matchCaseSensitivity() const;
    void setMatchCaseSensitivity( Qt::CaseSensitivity cs );

    static QString columnName(int column);
    int translateColumn(int sourceColumn) const;

    static QString propertyName(KTextEditor::CodeCompletionModel::CompletionProperty property);

    bool indexIsCompletion(const QModelIndex& index) const;

    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
    virtual bool hasIndex ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    virtual QMap<int, QVariant> itemData ( const QModelIndex & index ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex sibling ( int row, int column, const QModelIndex & index ) const;
    virtual void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    // Sorting
    bool isSortingEnabled() const;
    bool isSortingAlphabetical() const;
    void setSortingAlphabetical(bool alphabetical);

    Qt::CaseSensitivity sortingCaseSensitivity() const;
    void setSortingCaseSensitivity(Qt::CaseSensitivity cs);

    bool isSortingReverse() const;
    void setSortingReverse(bool reverse);

    // Filtering
    bool isFilteringEnabled() const;

    bool filterContextMatchesOnly() const;
    void setFilterContextMatchesOnly(bool filter);

    bool filterByAttribute() const;
    void setFilterByAttribute(bool filter);

    KTextEditor::CodeCompletionModel::CompletionProperties filterAttributes() const;
    void setFilterAttributes(KTextEditor::CodeCompletionModel::CompletionProperties attributes);

    // A maximum depth of <= 0 equals don't filter by inheritance depth (i.e. infinity) and is default
    int maximumInheritanceDepth() const;
    void setMaximumInheritanceDepth(int maxDepth);

    // Grouping
    bool isGroupingEnabled() const;

    enum gm {
      ScopeType     = 0x1,
      Scope         = 0x2,
      AccessType    = 0x4,
      ItemType      = 0x8
    };
    Q_DECLARE_FLAGS(GroupingMethods, gm)

    static const int ScopeTypeMask = 0x380000;
    static const int AccessTypeMask = 0x7;
    static const int ItemTypeMask = 0xfe0;

    GroupingMethods groupingMethod() const;
    void setGroupingMethod(GroupingMethods m);

    bool accessIncludeConst() const;
    void setAccessIncludeConst(bool include);
    bool accessIncludeStatic() const;
    void setAccessIncludeStatic(bool include);
    bool accessIncludeSignalSlot() const;
    void setAccessIncludeSignalSlot(bool include);

    // Column merging
    bool isColumnMergingEnabled() const;

    const QList< QList<int> >& columnMerges() const;
    void setColumnMerges(const QList< QList<int> >& columnMerges);

  public slots:
    void setSortingEnabled(bool enable);
    void setFilteringEnabled(bool enable);
    void setGroupingEnabled(bool enable);
    void setColumnMergingEnabled(bool enable);

  private slots:
    void slotRowsInserted( const QModelIndex & parent, int start, int end );
    void slotRowsRemoved( const QModelIndex & parent, int start, int end );

  private:
    // Represents a source row; provides sorting method
    class Item {
      public:
        Item(KateCompletionModel* model, int sourceRow);

        bool isValid() const;
        // Returns true if the item is not filtered and matches the current completion string
        bool isVisible() const;
        // Returns whether the item is filtered or not
        bool isFiltered() const;
        // Returns whether the item matches the current completion string
        bool isMatching() const;

        bool filter();
        bool match(const QString& newCompletion = QString());

        int sourceRow() const;

        // Sorting operator
        bool operator<(const Item& rhs) const;

      private:
        KateCompletionModel* model;
        int m_sourceRow;

        // True when currently matching completion string
        bool matchCompletion;
        // True when passes all active filters
        bool matchFilters;

        QString completionName() const;
    };

    // Grouping and sorting of rows
    class Group {
      public:
        explicit Group(KateCompletionModel* model);

        void addItem(Item i);
        void resort();
        void refilter();
        void clear();

        KateCompletionModel* model;
        int attribute;
        QString title, scope;
        QList<int> rows;
        QList<Item> prefilter;
        bool isEmpty;
    };

    void createGroups();
    void clearGroups();
    void hideOrShowGroup(Group* g);
    Group* ungrouped();
    Group* fetchGroup(int attribute, const QString& scope = QString());
    Group* groupForIndex(const QModelIndex& index) const;
    inline Group* groupOfParent(const QModelIndex& child) const { return static_cast<Group*>(child.internalPointer()); }
    QModelIndex indexForRow(Group* g, int row) const;
    QModelIndex indexForGroup(Group* g) const;

    enum changeTypes {
      Broaden,
      Narrow,
      Change
    };
    void changeCompletions(Group* g, const QString& newCompletion, changeTypes changeType);

    void deleteRows(Group* g, QMutableListIterator<int>& filtered, int countBackwards, int startRow);
    void addRows(Group* g, QMutableListIterator<int>& filtered, int startRow, const QList<int>& newItems);

    bool hasGroups() const;
    bool hasCompletionModel() const;

    /// Removes attributes not used in grouping from the input \a attribute
    int groupingAttributes(int attribute) const;
    int countBits(int value) const;

    void resort();
    void refilter();

    // ### Runtime state
    // General
    QString m_currentMatch;
    Qt::CaseSensitivity m_matchCaseSensitivity;

    bool m_hasCompletionModel;

    // Column merging
    QList< QList<int> > m_columnMerges;

    Group* m_ungrouped;
    bool m_ungroupedDisplayed;

    // Storing the sorted order
    QList<Group*> m_rowTable;
    QList<Group*> m_emptyGroups;
    // Quick access to each specific group (if it exists)
    QMultiHash<int, Group*> m_groupHash;

    // ### Configurable state
    // Sorting
    bool m_sortingEnabled;
    bool m_sortingAlphabetical;
    Qt::CaseSensitivity m_sortingCaseSensitivity;
    bool m_sortingReverse;
    QHash< int, QList<int> > m_sortingGroupingOrder;

    // Filtering
    bool m_filteringEnabled;
    bool m_filterContextMatchesOnly;
    bool m_filterByAttribute;
    KTextEditor::CodeCompletionModel::CompletionProperties m_filterAttributes;
    int m_maximumInheritanceDepth;

    // Grouping
    bool m_groupingEnabled;
    GroupingMethods m_groupingMethod;
    bool m_accessConst, m_accessStatic, m_accesSignalSlot;

    // Column merging
    bool m_columnMergingEnabled;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KateCompletionModel::GroupingMethods)

#endif
