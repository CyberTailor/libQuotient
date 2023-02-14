// SPDX-FileCopyrightText: 2018 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "roomevent.h"

#include "logging.h"
#include "redactionevent.h"

using namespace Quotient;

RoomEvent::RoomEvent(const QJsonObject& json) : Event(json)
{
    if (const auto redaction = unsignedPart<QJsonObject>(RedactedCauseKeyL);
        !redaction.isEmpty())
        _redactedBecause = loadEvent<RedactionEvent>(redaction);
}

RoomEvent::~RoomEvent() = default; // Let the smart pointer do its job

QString RoomEvent::id() const { return fullJson()[EventIdKeyL].toString(); }

QDateTime RoomEvent::originTimestamp() const
{
    return Quotient::fromJson<QDateTime>(fullJson()["origin_server_ts"_ls]);
}

QString RoomEvent::roomId() const
{
    return fullJson()[RoomIdKeyL].toString();
}

QString RoomEvent::senderId() const
{
    return fullJson()[SenderKeyL].toString();
}

QString RoomEvent::redactionReason() const
{
    return isRedacted() ? _redactedBecause->reason() : QString {};
}

QString RoomEvent::transactionId() const
{
    return unsignedPart<QString>("transaction_id"_ls);
}

QString RoomEvent::stateKey() const
{
    return fullJson()[StateKeyKeyL].toString();
}

void RoomEvent::setRoomId(const QString& roomId)
{
    editJson().insert(RoomIdKeyL, roomId);
}

void RoomEvent::setSender(const QString& senderId)
{
    editJson().insert(SenderKeyL, senderId);
}

void RoomEvent::setTransactionId(const QString& txnId)
{
    auto unsignedData = fullJson()[UnsignedKeyL].toObject();
    unsignedData.insert(QStringLiteral("transaction_id"), txnId);
    editJson().insert(UnsignedKeyL, unsignedData);
    Q_ASSERT(transactionId() == txnId);
}

void RoomEvent::addId(const QString& newId)
{
    Q_ASSERT(id().isEmpty());
    Q_ASSERT(!newId.isEmpty());
    editJson().insert(EventIdKeyL, newId);
    qCDebug(EVENTS) << "Event txnId -> id:" << transactionId() << "->" << id();
    Q_ASSERT(id() == newId);
}

void RoomEvent::dumpTo(QDebug dbg) const
{
    Event::dumpTo(dbg);
    dbg << " (made at " << originTimestamp().toString(Qt::ISODate) << ')';
}

#ifdef Quotient_E2EE_ENABLED
void RoomEvent::setOriginalEvent(event_ptr_tt<RoomEvent>&& originalEvent)
{
    _originalEvent = std::move(originalEvent);
}

const QJsonObject RoomEvent::encryptedJson() const
{
    if(!_originalEvent) {
        return {};
    }
    return _originalEvent->fullJson();
}
#endif
