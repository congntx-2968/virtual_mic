#ifndef LABELBUTTON_H
#define LABELBUTTON_H


#include <QLabel>
#include <QMouseEvent>

class LabelButton : public QLabel {
    Q_OBJECT

public:
    LabelButton(QWidget *parent = nullptr);
signals:
    void clicked();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QColor m_originalColor;
    QColor m_hoverColor;
};


#endif // LABELBUTTON_H
