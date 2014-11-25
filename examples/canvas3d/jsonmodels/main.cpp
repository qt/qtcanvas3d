#define NO_ENTERPRISE

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
#ifdef NO_ENTERPRISE
    engine.load(QUrl(QStringLiteral("qrc:///jsonmodelsbasic.qml")));
#else
    engine.load(QUrl(QStringLiteral("qrc:///jsonmodels.qml")));
#endif

    return app.exec();
}
