#include <QObject>
#include <QTimer>

class MainLoop : public QObject {
    Q_OBJECT
private:
    QTimer _timer;
private slots:
    void Update();
public:
    MainLoop();

    void Enter();
};