#include "DeclarativeInputEngine.h"

#include <QDebug>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QMetaEnum>
#include <QTimer>

/**
 * Private data class
 */
struct DeclarativeInputEnginePrivate {
    explicit DeclarativeInputEnginePrivate(DeclarativeInputEngine *_public);

    DeclarativeInputEngine *_this;
    bool Animating;
    QTimer *AnimatingFinishedTimer{nullptr};
    int InputMode;
    QRect KeyboardRectangle;

    bool isUppercase{false};
    bool symbolMode{false};
    QHash<QString, QString> layoutFiles = {
        {"En", "EnLayout"},         {"Fr", "FrLayout"},
        {"It", "ItLayout"},         {"Es", "EsLayout"},
        {"De", "DeLayout"},         {"Nl", "NlLayout"},
        {"Pt", "PtLayout"},         {"Cs", "CsLayout"},
        {"El", "ElLayout"},         {"Pl", "PlLayout"},
        {"Hr", "LtSrHrBsLayout"},   {"CyBs", "CySrBsLayout"},
        {"LtBs", "LtSrHrBsLayout"}, {"CySr", "CySrBsLayout"},
        {"LtSr", "LtSrHrBsLayout"}};
    // only needed for layouts with shared files
    QHash<QString, QString> layoutDescriptions = {{"Hr", "Hrvatski"},
                                                  {"CyBs", "Босански"},
                                                  {"LtBs", "Bosanski "},
                                                  {"CySr", "Српски"},
                                                  {"LtSr", "Srpski"}};
};

DeclarativeInputEnginePrivate::DeclarativeInputEnginePrivate(
    DeclarativeInputEngine *_public)
    : _this(_public),
      Animating(false),
      InputMode(DeclarativeInputEngine::Letters) {}

DeclarativeInputEngine::DeclarativeInputEngine(QObject *parent)
    : QObject(parent), d(new DeclarativeInputEnginePrivate(this)) {
    d->AnimatingFinishedTimer = new QTimer(this);
    d->AnimatingFinishedTimer->setSingleShot(true);
    d->AnimatingFinishedTimer->setInterval(100);
    connect(d->AnimatingFinishedTimer, &QTimer::timeout, this,
            &DeclarativeInputEngine::animatingFinished);
}

DeclarativeInputEngine::~DeclarativeInputEngine() { delete d; }

bool DeclarativeInputEngine::virtualKeyClick(Qt::Key key, const QString &text,
                                             Qt::KeyboardModifiers modifiers) {
    QKeyEvent pressEvent(QEvent::KeyPress, key,
                         Qt::KeyboardModifiers(modifiers), text);
    QKeyEvent releaseEvent(QEvent::KeyRelease, key,
                           Qt::KeyboardModifiers(modifiers), text);
    return QCoreApplication::sendEvent(QGuiApplication::focusObject(),
                                       &pressEvent) &&
           QCoreApplication::sendEvent(QGuiApplication::focusObject(),
                                       &releaseEvent);
}

QRect DeclarativeInputEngine::keyboardRectangle() const {
    return d->KeyboardRectangle;
}

void DeclarativeInputEngine::setKeyboardRectangle(const QRect &Rect) {
    setAnimating(true);
    d->AnimatingFinishedTimer->start(100);
    d->KeyboardRectangle = Rect;
    emit keyboardRectangleChanged();
}

bool DeclarativeInputEngine::isAnimating() const { return d->Animating; }

void DeclarativeInputEngine::setAnimating(bool Animating) {
    if (d->Animating != Animating) {
        d->Animating = Animating;
        emit animatingChanged();
    }
}

void DeclarativeInputEngine::animatingFinished() { setAnimating(false); }

int DeclarativeInputEngine::inputMode() const { return d->InputMode; }

void DeclarativeInputEngine::setInputMode(int Mode) {
    if (Mode != d->InputMode) {
        d->InputMode = Mode;
        emit inputModeChanged();
    }
}

bool DeclarativeInputEngine::isUppercase() const { return d->isUppercase; }

void DeclarativeInputEngine::setUppercase(bool uppercase) {
    if (d->isUppercase != uppercase) {
        d->isUppercase = uppercase;
        emit isUppercaseChanged();
    }
}

bool DeclarativeInputEngine::isSymbolMode() const { return d->symbolMode; }

void DeclarativeInputEngine::setSymbolMode(bool symbolMode) {
    if (d->symbolMode != symbolMode) {
        d->symbolMode = symbolMode;
        emit isSymbolModeChanged();
    }
}

bool DeclarativeInputEngine::inputLayoutValid(const QString &layout) const {
    for (int i = InputLayouts::En; i != InputLayouts::EndLayouts; i++) {
        const auto validLayout = QMetaEnum::fromType<InputLayouts>().valueToKey(
            static_cast<InputLayouts>(i));
        if (validLayout == layout) {
            return true;
        }
    }

    qCritical() << "Keyboard layout" << layout
                << "is not supported. Falling back to En!";
    return false;
}

QString DeclarativeInputEngine::getFileOfLayout(QString layout) {
    QHash<QString, QString>::const_iterator found =
        d->layoutFiles.constFind(layout);
    if (found != d->layoutFiles.cend()) {
        return found.value();
    }
    return "";
}

QString DeclarativeInputEngine::getDescriptionOfLayout(QString layout) {
    QHash<QString, QString>::const_iterator found =
        d->layoutDescriptions.constFind(layout);
    if (found != d->layoutDescriptions.cend()) {
        return found.value();
    }
    return "";
}
