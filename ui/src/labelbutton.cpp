#include "labelbutton.h"

LabelButton::LabelButton(QWidget *parent) : QLabel(parent),
    m_originalColor(244, 70, 72),
    m_hoverColor(255, 140, 142) {

    // Set the original color
    setStyleSheet(QString("color: %1;").arg(m_originalColor.name()));
}

void LabelButton::enterEvent(QEvent *event) {
    // Change the cursor to a hand cursor
    setCursor(Qt::PointingHandCursor);

    // Change the color to the hover color
//    setStyleSheet(QString("color: rgb(%1, %2, %3);").arg(m_hoverColor.red()).arg(m_hoverColor.green()).arg(m_hoverColor.blue()));
    setStyleSheet(QString("color: %1;").arg(m_hoverColor.name()));

    QLabel::enterEvent(event);
}

void LabelButton::leaveEvent(QEvent *event) {
    // Reset the cursor
    unsetCursor();

    // Change the color back to the original color
//    setStyleSheet(QString("color: rgb(%1, %2, %3);").arg(m_originalColor.red()).arg(m_originalColor.green()).arg(m_originalColor.blue()));
    setStyleSheet(QString("color: %1;").arg(m_originalColor.name()));

    QLabel::leaveEvent(event);
}

void LabelButton::mouseReleaseEvent(QMouseEvent* event){
    emit clicked();
}
