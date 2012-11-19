#include "run.h"
#include "stdlib/kumstdlib.h"
#define DO_NOT_DECLARE_STATIC
#include "stdlib/kumirstdlib.hpp"

namespace KumirCodeRun {

Run::Run(QObject *parent) :
    QThread(parent)
{
    programLoaded = false;
    vm = new VM(this);

    i_originFunctionDeep = 0;
    b_interactDone = b_stopping = b_stepDone = b_algDone = false;
    mutex_stopping = new QMutex;
    mutex_stepDone = new QMutex;
    mutex_algDone = new QMutex;
    mutex_interactDone = new QMutex;
    e_runMode = RM_ToEnd;
    connect(vm, SIGNAL(pauseRequest()), this, SLOT(handlePauseRequest()), Qt::DirectConnection);

    connect(vm, SIGNAL(lineNoChanged(int)), this, SLOT(handleLineChanged(int)), Qt::DirectConnection);
    connect(vm, SIGNAL(retInstruction(int)), this, SLOT(handleAlgorhitmDone(int)), Qt::DirectConnection);

    connect(vm, SIGNAL(inputRequest(QString,QList<quintptr>,QList<int>)),
            this, SLOT(handleInputRequest(QString,QList<quintptr>,QList<int>)), Qt::DirectConnection);
    connect(vm, SIGNAL(inputArgumentRequest(int,QString,QString,QList<int>)),
            this, SLOT(handleInputArgumentRequest(int,QString,QString,QList<int>)), Qt::DirectConnection);
    connect(vm, SIGNAL(outputArgumentRequest(QVariant,QString,QList<int>)),
            this, SLOT(handleOutputArgumentRequest(QVariant,QString,QList<int>)),
            Qt::DirectConnection);
    connect(vm, SIGNAL(outputRequest(QStringList, QList<QVariant::Type>, QVariantList)),
                       this, SLOT(handleOutputRequest(QStringList, QList<QVariant::Type>, QVariantList)), Qt::DirectConnection);
//    connect(vm, SIGNAL(invokeExternalFunction(QString,QString,QVariantList,QList<quintptr>,QList<int>)),
//            this, SLOT(handleExternalRequest(QString,QString,QVariantList,QList<quintptr>,QList<int>)),
//            Qt::DirectConnection);
    connect(vm, SIGNAL(invokeExternalFunction(QList<quintptr>)),
            this, SLOT(handleExternalRequest(QList<quintptr>)),
            Qt::DirectConnection);
    connect(vm, SIGNAL(beforeExternalFunction()),
            this, SLOT(prepareExternalCall()), Qt::DirectConnection);
    connect(vm, SIGNAL(resetModuleRequest(QString)), this, SIGNAL(resetModule(QString)));

}

void Run::stop()
{
    QMutexLocker l(mutex_stopping);
    b_stopping = true;
    if (!isRunning()) {
        emit lineChanged(-1);
        emit finished();
    }
}

void Run::runStepOver()
{
    b_stepDone = false;
    b_stopping = false;
//    i_originFunctionDeep = vm->deep();
    e_runMode = RM_StepOver;
    vm->setNextCallStepOver();
    start();
}

void Run::runStepIn()
{
    b_stepDone = false;
//    i_originFunctionDeep = vm->deep();
    e_runMode = RM_StepIn;
    vm->setNextCallInto();
    start();
}

void Run::runStepOut()
{
    b_stepDone = false;
    b_algDone = false;
//    i_originFunctionDeep = vm->deep();
    emit lineChanged(-1);
    e_runMode = RM_StepOut;
    vm->setNextCallOut();
    start();
}

void Run::runBlind()
{
    e_runMode = RM_ToEnd;
    vm->setBlindMode(true);
    vm->setNextCallToEnd();
    start();
}

void Run::runContinuous()
{
    e_runMode = RM_ToEnd;
    b_stopping = false;
    vm->setNextCallToEnd();
    start();
}


void Run::handleInputRequest(const QString & format, const QList<quintptr> & references, const QList<int> & indeces)
{
    mutex_interactDone->lock();
    b_interactDone = false;
    list_inputResult.clear();
    mutex_interactDone->unlock();
    QVariantList result;
    emit input(format);
    forever {
        mutex_interactDone->lock();
        result = list_inputResult;
        mutex_interactDone->unlock();
        if (result.isEmpty()) {
            msleep(50);
        }
        else {
            break;
        }
        if (mustStop())
            break;
    }
    if (mustStop())
        return;
    Q_ASSERT(result.size()==references.size());
    vm->setResults("", references, indeces, result);
}

void Run::handleInputArgumentRequest(int localId,
                                     const QString &varName,
                                     const QString &baseFormat,
                                     const QList<int> &bounds)
{
    QVariantList result;
    QVariantList localResult;
    int totalItems = 1;
    int currentIndex = 0;
    int dimension = 0;
    bool inputNext = true;
    int z = 0;
    int y = 0;
    int x = 0;
    int size0 = 0;
    int size1 = 0;
    int size2 = 0;
    if (bounds.size()==2) {
        dimension = 1;
        size0 = bounds[1]-bounds[0]+1;
        x = bounds[0];
        totalItems = size0;
    }
    else if (bounds.size()==4) {
        dimension = 2;
        size0 = bounds[3]-bounds[2]+1;
        size1 = bounds[1]-bounds[0]+1;
        x = bounds[0];
        y = bounds[2];
        totalItems = size0 * size1;
    }
    else if (bounds.size()==6) {
        dimension = 3;
        size0 = bounds[5]-bounds[4]+1;
        size1 = bounds[3]-bounds[2]+1;
        size2 = bounds[1]-bounds[0]+1;
        x = bounds[0];
        y = bounds[2];
        z = bounds[4];
        totalItems = size0 * size1 * size2;
    }
    forever {
        if (inputNext) {
            inputNext = false;
            mutex_interactDone->lock();
            b_interactDone = false;
            list_inputResult.clear();
            mutex_interactDone->unlock();
            QString varNameAndIndeces = varName;
            if (dimension>0) {
                QStringList indeces;
                indeces.prepend(QString::number(x+currentIndex%size0));
                if (dimension>=2)
                    indeces.prepend(QString::number(y+currentIndex%(size0*size1)));
                if (dimension==3)
                    indeces.prepend(QString::number(z+currentIndex%(size0*size1*size2)));
                varNameAndIndeces += "["+indeces.join(",")+"]";
            }
            QString greeting = tr("Please enter %1: ").arg(varNameAndIndeces);
            emit output(greeting);
            emit input(baseFormat);
        }
        mutex_interactDone->lock();
        localResult = list_inputResult;
        mutex_interactDone->unlock();
        if (localResult.isEmpty()) {
            msleep(50);
        }
        else {
            result << localResult.at(0);
            currentIndex++;
            inputNext = true;
            if (totalItems==currentIndex) {
                break;
            }
        }
        if (mustStop())
            break;
    }
    if (mustStop())
        return;
    if (dimension==0) {
        vm->setLocalVariableValue(localId, result.first());
    }
    else {
        vm->setLocalVariableValue(localId, QVariant(result));
    }
}

void Run::handleOutputArgumentRequest(const QVariant & value,
                                     const QString &varName,
                                     const QList<int> &bounds)
{
    if (value.type()!=QVariant::List) {
        QString out = varName + "=";
        if (!value.isValid())
            out += tr("undefined");
        else if (value.type()==QVariant::String)
            out += "\""+value.toString()+"\"";
        else if (value.type()==QVariant::Char)
            out += "'"+value.toString()+"'";
        else
            out += value.toString();
        out += "\n";
        emit output(out);
    }
    else {
        const QVariantList & list = value.toList();
        int totalItems = 1;
        int currentIndex = 0;
        int dimension = 0;
        int z = 0;
        int y = 0;
        int x = 0;
        int size0 = 0;
        int size1 = 0;
        int size2 = 0;
        if (bounds.size()==2) {
            dimension = 1;
            size0 = bounds[1]-bounds[0]+1;
            x = bounds[0];
            totalItems = size0;
        }
        else if (bounds.size()==4) {
            dimension = 2;
            size0 = bounds[3]-bounds[2]+1;
            size1 = bounds[1]-bounds[0]+1;
            x = bounds[0];
            y = bounds[2];
            totalItems = size0 * size1;
        }
        else if (bounds.size()==6) {
            dimension = 3;
            size0 = bounds[5]-bounds[4]+1;
            size1 = bounds[3]-bounds[2]+1;
            size2 = bounds[1]-bounds[0]+1;
            x = bounds[0];
            y = bounds[2];
            z = bounds[4];
            totalItems = size0 * size1 * size2;
        }
        for ( ; currentIndex<totalItems; currentIndex ++) {
            QString varNameAndIndeces = varName;
            if (dimension>0) {
                QStringList indeces;
                indeces.prepend(QString::number(x+currentIndex%size0));
                if (dimension>=2)
                    indeces.prepend(QString::number(y+currentIndex%(size0*size1)));
                if (dimension==3)
                    indeces.prepend(QString::number(z+currentIndex%(size0*size1*size2)));
                varNameAndIndeces += "["+indeces.join(",")+"]";
            }
            QString out = varNameAndIndeces + "=";
            const QVariant & val = list[currentIndex];
            if (!val.isValid())
                out += tr("undefined");
            else if (val.type()==QVariant::String)
                out += "\""+val.toString()+"\"";
            else if (val.type()==QVariant::Char)
                out += "'"+val.toString()+"'";
            else
                out += val.toString();
            out += "\n";
            emit output(out);
        }
    }
}

void Run::handleExternalRequest(const QString &pluginName,
                                const QString &functionName,
                                const QVariantList &arguments,
                                const QList<quintptr> &references,
                                const QList<int> &indeces)
{
    mutex_interactDone->lock();
    b_interactDone = false;
    list_funcResults.clear();
    v_funcResult = QVariant::Invalid;
    s_funcError = "";
    mutex_interactDone->unlock();
    QVariantList result;
    emit externalFunctionCall(pluginName, functionName, arguments);
    bool done = false;
    forever {
        mutex_interactDone->lock();
        done = b_interactDone;
        mutex_interactDone->unlock();
        if (!done) {
            msleep(1);
        }
        else {
            break;
        }
        if (mustStop())
            break;
    }
    if (mustStop())
        return;
    Q_ASSERT(result.size()==references.size());
    vm->pushValueToStack(v_funcResult);
    vm->setResults(s_funcError, references, indeces, list_funcResults);
}

void Run::prepareExternalCall()
{
    mutex_interactDone->lock();
    b_interactDone = false;
    mutex_interactDone->unlock();
}

void Run::handleExternalRequest(const QList<quintptr> &references)
{
    bool done = false;
    forever {
        mutex_interactDone->lock();
        done = b_interactDone;
        mutex_interactDone->unlock();
        if (!done) {
            msleep(1);
        }
        else {
            break;
        }
        if (mustStop())
            break;
    }
    if (mustStop())
        return;
    Q_ASSERT(list_funcResults.size()==references.size());
    if (v_funcResult.isValid())
        vm->pushValueToStack(v_funcResult);
    v_funcResult = QVariant::Invalid;
    vm->setResults(s_funcError, references, QList<int>(), list_funcResults);
    list_funcResults.clear();
    b_interactDone = false;
    s_funcError.clear();
}

void Run::finishInput(const QVariantList &data)
{
    QMutexLocker l(mutex_interactDone);
    b_interactDone = true;
    list_inputResult = data;
}

void Run::finishExternalFunctionCall(const QString & error,
                                     const QVariant &retval,
                                     const QVariantList &results)
{
    QMutexLocker l(mutex_interactDone);
    b_interactDone = true;
    list_funcResults = results;
    v_funcResult = retval;
    s_funcError = error;
}

void Run::handleOutputRequest(const QStringList &formats, const QList<QVariant::Type> & types, const QVariantList & values)
{
    Kumir::IO::OutputStream os;
    for (int i=0; i<formats.size(); i++) {
        Kumir::String format = formats[i].toStdWString();
        if (types[i]==QVariant::Int) {
            Kumir::IO::IntFormat fmt = Kumir::IO::parseIntFormat(format);
            if (Kumir::Core::getError().length()>0) return;
            Kumir::IO::writeInteger(os, values[i].toInt(), fmt);
        }
        else if (types[i]==QVariant::Double) {
            Kumir::IO::RealFormat fmt = Kumir::IO::parseRealFormat(format);
            if (Kumir::Core::getError().length()>0) return;
            Kumir::IO::writeReal(os, values[i].toDouble(), fmt);
        }
        else if (types[i]==QVariant::Bool) {
            Kumir::IO::BoolFormat fmt = Kumir::IO::parseBoolFormat(format);
            if (Kumir::Core::getError().length()>0) return;
            Kumir::IO::writeBool(os, values[i].toBool(), fmt);
        }
        else if (types[i]==QVariant::Char) {
            Kumir::IO::CharFormat fmt = Kumir::IO::parseCharFormat(format);
            if (Kumir::Core::getError().length()>0) return;
            Kumir::IO::writeChar(os, values[i].toChar().unicode(), fmt);
        }
        else if (types[i]==QVariant::String) {
            Kumir::IO::StringFormat fmt = Kumir::IO::parseStringFormat(format);
            if (Kumir::Core::getError().length()>0) return;
            Kumir::IO::writeString(os, values[i].toString().toStdWString(), fmt);
        }
    }
    QString data = QString::fromStdWString(os.getBuffer());
    emit output(data);
}

bool Run::mustStop()
{
    QMutexLocker l1(mutex_stopping);
    QMutexLocker l2(mutex_stepDone);

    if (vm->error().length()>0)
        return true;

    if (b_stopping)
        return true;

    if (e_runMode==RM_StepOut) {
        return b_algDone;
    }
    else if (e_runMode!=RM_ToEnd) {
        return b_stepDone;
    }
    else {
        return false;
    }
}

void Run::handlePauseRequest()
{
    e_runMode = RM_StepOver;
}

void Run::handleAlgorhitmDone(int lineNo)
{
    mutex_algDone->lock();
    b_algDone = true;
    mutex_algDone->unlock();
    if (mustStop())
        emit lineChanged(lineNo);
    else
        emit lineChanged(-1);
}

void Run::handleLineChanged(int lineNo)
{
    mutex_stepDone->lock();
    b_stepDone = true;
    mutex_stepDone->unlock();
    if (mustStop())
        emit lineChanged(lineNo);
    else
        emit lineChanged(-1);

}

void Run::run()
{
    while (vm->hasMoreInstructions()) {
        if (mustStop()) {
            break;
        }
        vm->evaluateNextInstruction();
        if (vm->error().length()>0) {
            emit lineChanged(vm->effectiveLineNo());
            emit error(QString::fromStdWString(vm->error()));
            break;
        }
    }
//    bool wasError = vm->error().length()>0;
    // Unclosed files is an error only if program reached end
    bool unclosedFilesIsNotError = b_stopping || vm->hasMoreInstructions();
    // Must close all files if program reached end or user terminated
    bool programFinished = b_stopping || !vm->hasMoreInstructions();
//    __check_for_unclosed_files__st_funct(unclosedFilesIsNotError, closeUnclosedFiles);
//    vm->updateStFunctError();
//    if (!wasError && vm->error().length()>0) {
//        emit lineChanged(vm->effectiveLineNo());
//        emit error(QString::fromStdWString(vm->error()));
//    }
    if (programFinished)
        Kumir::finalizeStandardLibrary();
    emit aboutToStop();
}

} // namespace KumirCodeRun
