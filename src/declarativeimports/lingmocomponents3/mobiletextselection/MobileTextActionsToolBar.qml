/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

pragma Singleton

import QtQuick
import org.kde.lingmoui as LingmoUI

Loader {
    property Item controlRoot: null
    property bool shouldBeVisible: false

    active: controlRoot ? shouldBeVisible && LingmoUI.Settings.tabletMode && (controlRoot.selectedText.length > 0 || controlRoot.canPaste) : false
    source: "MobileTextActionsToolBarImpl.qml"
}
