// SPDX-FileCopyrightText: 2018 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "event.h"

namespace Quotient {
class QUOTIENT_API DirectChatEvent : public Event {
public:
    QUO_EVENT(DirectChatEvent, "m.direct")

    using Event::Event;

    QMultiHash<QString, QString> usersToDirectChats() const;
};
} // namespace Quotient
