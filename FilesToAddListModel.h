#ifndef FILESTOADDLISTMODEL_H
#define FILESTOADDLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <FilesToAdd.h>
#include <vector>

class FilesToAddListModel : public QAbstractListModel {

    Q_OBJECT

public:
    explicit FilesToAddListModel(QStringList filepaths, QObject* parent = 0);
    void setStringList(QStringList filepaths, const QModelIndex& parent = QModelIndex());
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const;
    void removeRow(int row, const QModelIndex& parent = QModelIndex());
    unsigned long get_sum_size();

private:
    std::vector<FilesToAdd> FilesToAdd_;
    unsigned long sum_size;
    void SubtractFromSum();

};

#endif // FILESTOADDLISTMODEL_H
