#include <QDebug>
#include <QUrl>

#include "control/controlobject.h"
#include "widget/wtrackproperty.h"
#include "util/dnd.h"

WTrackProperty::WTrackProperty(const char* group,
                               UserSettingsPointer pConfig,
                               QWidget* pParent,
                               TrackCollectionManager* pTrackCollectionManager)
        : WLabel(pParent),
          m_pGroup(group),
          m_pConfig(pConfig) {
    setAcceptDrops(true);

    // Setup context menu
    WTrackMenu::Features flags = WTrackMenu::Feature::Playlist |
            WTrackMenu::Feature::Crate |
            WTrackMenu::Feature::Metadata |
            WTrackMenu::Feature::Reset |
            WTrackMenu::Feature::BPM |
            WTrackMenu::Feature::Color |
            WTrackMenu::Feature::FileBrowser |
            WTrackMenu::Feature::Properties;
    m_pMenu = new WTrackMenu(this, pConfig, pTrackCollectionManager, flags);
}

void WTrackProperty::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);

    m_property = context.selectString(node, "Property");
}

void WTrackProperty::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.get(),
                &Track::changed,
                this,
                &WTrackProperty::slotTrackChanged);
        updateLabel();
    }
}

void WTrackProperty::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    updateLabel();
}

void WTrackProperty::slotTrackChanged(TrackId trackId) {
    Q_UNUSED(trackId);
    updateLabel();
}

void WTrackProperty::updateLabel() {
    if (m_pCurrentTrack) {
        QVariant property = m_pCurrentTrack->property(m_property.toUtf8().constData());
        if (property.isValid() && property.canConvert(QMetaType::QString)) {
            setText(property.toString());
            return;
        }
    }
    setText("");
}

void WTrackProperty::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_pGroup);
    }
}

void WTrackProperty::dragEnterEvent(QDragEnterEvent *event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_pGroup, m_pConfig);
}

void WTrackProperty::dropEvent(QDropEvent *event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_pGroup, m_pConfig);
}

void WTrackProperty::contextMenuEvent(QContextMenuEvent *event) {
    if (m_pCurrentTrack) {
        m_pMenu->loadTrack(m_pCurrentTrack->getId());
        // Create the right-click menu
        m_pMenu->popup(event->globalPos());
    }
}
