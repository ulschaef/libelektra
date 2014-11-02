#ifndef CONFIGNODE_H
#define CONFIGNODE_H

#include <QObject>
#include <QVariant>
#include <kdb.hpp>
#include <keyio.hpp>
#include <cassert>

#include "treeviewmodel.hpp"
#include "printvisitor.hpp"

class TreeViewModel;
class PrintVisitor;

class ConfigNode : public QObject
{
    Q_OBJECT

public:

    explicit ConfigNode(const QString& name, const QString& path, const kdb::Key &key, TreeViewModel *parentModel);
    /// Needed by Qt. This copy constructor is supposed to create a DEEP COPY.
    ConfigNode(const ConfigNode& other);
    /// Needed by Qt
    ConfigNode();
    ~ConfigNode();

    /**
     * @brief Returns the number of children of this ConfigNode.
     * @return The number of children of this ConfigNode.
     */
    int                         getChildCount() const;

    /**
     * @brief Returns the name of this ConfigNode.
     * @return The name of this ConfigNode.
     */
    QString                     getName() const;

    /**
     * @brief Returns the path of this ConfigNode.
     * @return The path of this ConfigNode.
     */
    QString                     getPath() const;

    /**
     * @brief Returns the value of this ConfigNode.
     * @return The value of this ConfigNode.
     */
    QVariant                    getValue() const;

    /**
     * @brief Rename this ConfigNode.
     * @param name The new name for this ConfigNode.
     */
    void                        setName(const QString& name);

    /**
     * @brief Change the value of this ConfigNode.
     * @param value The new value for this ConfigNode.
     */
    void                        setValue(const QVariant& value);

    /**
     * @brief Append a new child to this ConfigNode.
     * @param node The new child of this ConfigNode.
     */
    void                        appendChild(ConfigNode* node);

    /**
     * @brief Returns if this ConfigNode has a child with a certain name.
     * @param name The name of the child node.
     * @return True if this node has a child with a certain name.
     */
    bool                        hasChild(const QString& name) const;

    /**
     * @brief Get the children of this ConfigNode.
     * @return The children of this ConfigNode as model.
     */
    TreeViewModel*              getChildren() const;

    /**
     * @brief Get the metakeys of this ConfigNode.
     * @return The metakeys of this ConfigNode as model.
     */
    TreeViewModel*              getMetaKeys() const;

    /**
     * @brief Returns if the children of this ConfigNode have any children themselves.
     * @return True if no child of this ConfigNode has any children.
     */
    bool                        childrenHaveNoChildren() const;

    /**
     * @brief Returns a child with a certain name.
     * @param name The name of the child which is looked for.
     * @return The child with the given name if it is a child of this ConfigNode.
     */
    ConfigNode*                 getChildByName(QString& name) const;

    /**
      * @brief Returns a child on a given index.
      * @param index The index of the wanted child.
      * @return The child on the given index.
      */
    Q_INVOKABLE ConfigNode*     getChildByIndex(int index) const;

    /**
     * @brief Change the path of this ConfigNode.
     * @param path The new path of this ConfigNode.
     */
    void                        setPath(const QString &path);

    /**
     * @brief Change or add a metakey of this ConfigNode. This method is used if this ConfigNode is a metakey.
     * @param name The name of the metakey.
     * @param value The value of the metakey.
     */
    void                        setMeta(const QString &name, const QVariant &value);

    /**
     * @brief Change or add more than one metakeys of this ConfigNode. This method is used if this ConfigNode is a metakey.
     * @param metaData The data consists of metanames and metavalues respectively.
     */
    Q_INVOKABLE void            setMeta(const QVariantMap &metaData);

    /**
     * @brief Delete a metakey of this ConfigNode. This method is used if this ConfigNode is a metakey.
     * @param name The name of the metakey that is supposed to be deleted.
     */
    Q_INVOKABLE void            deleteMeta(const QString &name);

    /**
     * @brief This method accepts a visitor to support the Vistor Pattern.
     * @param visitor The Visitor that visits this ConfigNode.
     */
    void                        accept(Visitor &visitor);

    /**
     * @brief Get the underlying Key of this ConfigNode.
     * @return The underlying Key of this ConfigNode.
     */
    kdb::Key                    getKey() const;

    /**
     * @brief Set the underlying Key of this ConfigNode.
     * @param key The new Key of this ConfigNode.
     */
    void                        setKey(kdb::Key key);

    /**
     * @brief Change the name of the underlying Key of this ConfigNode.
     * @param name The new name of the underlying Key of this ConfigNode.
     */
    void                        setKeyName(const QString &name);

    /**
     * @brief This method is needed to support undoing the creation of a new ConfigNode. Since a new ConfigNode is added via the @see TreeViewModel#sink method, it
     * is not possible to say where the new ConfigNode will find its place. This method is supposed to "clean up" the path of the new ConfigNode, if the insertion
     * is going to be reverted.
     * @param path The whole path of the ConfigNode which should be removed.
     */
    void                        deletePath(QStringList &path);

    /**
     * @brief Returns the index of a child ConfigNode, based on its name; if there is no child with this name, the return index is -1
     * @param name The name of this ConfigNode.
     * @return The index of this ConfigNode.
     */
    int                         getChildIndexByName(const QString &name);

    /**
     * @brief Returns a pointer to the TreeViewModel this ConfigNode is in.
     * @return A pointer to the TreeViewModel this ConfigNode is in.
     */
    TreeViewModel*              getParentModel();

    /**
     * @brief Sets a pointer to the TreeViewModel this ConfigNode is in.
     * @param parent The TreeViewModel this ConfigNode is in.
     */
    void                        setParentModel(TreeViewModel *parent);
    void                        clear();
    bool                        getIsExpanded() const;

private:
    // TODO: not needed if we hold the Key
    QString         m_name;
    QString         m_path;
    QVariant        m_value;

    // that is the only part we need:
    kdb::Key        m_key;
    TreeViewModel*  m_children;
    TreeViewModel*  m_metaData;
    TreeViewModel*  m_parentModel;

    bool            m_isExpanded;



    /**
     * @brief Populates the TreeViewModel which holds the metakeys of this ConfigNode.
     *
     */
    void populateMetaModel();

signals:
    void showMessage(QString title, QString text, QString informativeText, QString detailedText, QString icon);

public slots:
    void setIsExpanded(bool value);
};

Q_DECLARE_METATYPE(ConfigNode)

#endif // CONFIGNODE_H