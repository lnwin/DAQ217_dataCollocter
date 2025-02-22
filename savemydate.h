#ifndef SAVEMYDATE_H
#define SAVEMYDATE_H

#include <QObject>
#include <QThread>
class saveMydate  : public QThread {
    Q_OBJECT
public:
    explicit saveMydate(QObject *parent = nullptr);
    QString filePath;

signals:
public slots:
    void getData2Save(const std::vector<std::vector<float>>& data);
};

#endif // SAVEMYDATE_H
