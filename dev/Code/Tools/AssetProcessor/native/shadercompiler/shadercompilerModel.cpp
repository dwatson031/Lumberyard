/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#include "shadercompilerModel.h"
#if !defined(BATCH_MODE)
#include <QClipboard>
#include <QApplication>
#endif

namespace
{
    ShaderCompilerModel* s_singleton = nullptr;
}

ShaderCompilerModel::ShaderCompilerModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    Q_ASSERT(s_singleton == nullptr);
    s_singleton = this;
}

ShaderCompilerModel::~ShaderCompilerModel()
{
    s_singleton = nullptr;
}

ShaderCompilerModel* ShaderCompilerModel::Get()
{
    return s_singleton;
}

QVariant ShaderCompilerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    int row = index.row();

    if (row < 0)
    {
        return QVariant();
    }
    if (row >= m_shaderErrorInfoList.count())
    {
        return QVariant();
    }

    switch (role)
    {
    case TimeStampRole:
        return m_shaderErrorInfoList[row].m_shaderTimestamp;
    case ServerRole:
        return m_shaderErrorInfoList[row].m_shaderServerName;
    case ErrorRole:
        return m_shaderErrorInfoList[row].m_shaderError;
    case OriginalRequestRole:
        return m_shaderErrorInfoList[row].m_shaderOriginalPayload;

    case Qt::DisplayRole:
        switch (index.column())
        {
        case ColumnTimeStamp:
            return m_shaderErrorInfoList[row].m_shaderTimestamp;
        case ColumnServer:
            return m_shaderErrorInfoList[row].m_shaderServerName;
        case ColumnError:
            return m_shaderErrorInfoList[row].m_shaderServerName;
        }
    }

    return QVariant();
}


Qt::ItemFlags ShaderCompilerModel::flags(const QModelIndex& index) const
{
    (void)index;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}


int ShaderCompilerModel::rowCount(const QModelIndex& parent) const
{
    (void)parent;
    return m_shaderErrorInfoList.count();
}


QModelIndex ShaderCompilerModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}


QModelIndex ShaderCompilerModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row >= rowCount(parent) || column >= columnCount(parent))
    {
        return QModelIndex();
    }
    return createIndex(row, column);
}


int ShaderCompilerModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : Column::Max;
}


QVariant ShaderCompilerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case ColumnTimeStamp:
            return tr("Time Stamp");
        case ColumnServer:
            return tr("Server");
        case ColumnError:
            return tr("Error");
        default:
            break;
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}


QHash<int, QByteArray> ShaderCompilerModel::roleNames() const
{
    QHash<int, QByteArray> result;
    result[TimeStampRole] = "timestamp";
    result[ServerRole] = "server";
    result[ErrorRole] = "error";
    result[OriginalRequestRole] = "originalRequest";
    return result;
}
void ShaderCompilerModel::addShaderErrorInfoEntry(QString errorMessage, QString timestamp, QString payload, QString server)
{
    ShaderCompilerErrorInfo shaderCompileErrorInfo(errorMessage, timestamp, payload, server);
    beginInsertRows(QModelIndex(), m_shaderErrorInfoList.size(), m_shaderErrorInfoList.size());
    m_shaderErrorInfoList.append(shaderCompileErrorInfo);
    endInsertRows();
}
#if !defined(BATCH_MODE)
void ShaderCompilerModel::copyToClipboard(int row)
{
    QString payload = data(index(row, 0, QModelIndex()), OriginalRequestRole).toString();
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(payload);
    clipboard = nullptr;
}
#endif

#include <native/shadercompiler/shadercompilerModel.moc>
