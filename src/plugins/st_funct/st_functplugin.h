#ifndef EDITOR_PLUGIN_H
#define EDITOR_PLUGIN_H

#include "extensionsystem/kplugin.h"
#include "interfaces/actorinterface.h"


using namespace Shared;

namespace st_funct {

class St_functPlugin
        : public ExtensionSystem::KPlugin
        , public Shared::ActorInterface

{
    Q_OBJECT
    Q_INTERFACES(Shared::ActorInterface)
public:
    St_functPlugin();
    ~St_functPlugin();

    QList<Alg>  funcList() const ; // List of public functions;
    QString name(ProgrammingLanguage pl = PL_Kumir, QLocale::Language nl = QLocale::Russian) const;
    QStringList actorLibraries() const;
    QStringList usedQtLibraries() const;
    inline bool requiresGui() const { return false; }
    QVariant     result() const;
    void   runFunct(const QString &alg, const QList<QVariant> &params);
    QList<QVariant> algOptResults() const;//optional results
    inline class QWidget* mainWidget() { return 0; }
    inline QList<class QAction*> menuActions() { return QList<class QAction*>(); }
    void reset();
    QString errorText() const;
public slots:
    // Begin function implementations

//    EvaluationResult Abs(const QVariant & x);
//    EvaluationResult ArcCos(const QVariant & x);
//    EvaluationResult ArcSin(const QVariant & x);
//    EvaluationResult ArcCtg(const QVariant & x);
//    EvaluationResult ArcCtg(const QVariant & x);
//    EvaluationResult ArcCtg(const QVariant & x);

    // End function implementations
signals:
    void sync();
protected:
    QString initialize(const QStringList &arguments);
    void start();
    void stop();
private:
    QString errText;
    QVariant Res;
    QList<QVariant> optResults;

};

}


#endif
