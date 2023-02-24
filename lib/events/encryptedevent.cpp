// SPDX-FileCopyrightText: 2019 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "encryptedevent.h"
#include "e2ee/e2ee_common.h"
#include "logging.h"

using namespace Quotient;

EncryptedEvent::EncryptedEvent(const QJsonObject& ciphertexts,
                               const QString& senderKey)
    : RoomEvent(basicJson(TypeId, { { AlgorithmKeyL, OlmV1Curve25519AesSha2AlgoKey },
                                    { CiphertextKeyL, ciphertexts },
                                    { SenderKeyKeyL, senderKey } }))
{}

EncryptedEvent::EncryptedEvent(const QByteArray& ciphertext,
                               const QString& senderKey,
                               const QString& deviceId, const QString& sessionId)
    : RoomEvent(basicJson(TypeId, { { AlgorithmKeyL, MegolmV1AesSha2AlgoKey },
                                    { CiphertextKeyL, QString::fromLatin1(ciphertext) },
                                    { DeviceIdKeyL, deviceId },
                                    { SenderKeyKeyL, senderKey },
                                    { SessionIdKeyL, sessionId } }))
{}

EncryptedEvent::EncryptedEvent(const QJsonObject& obj)
    : RoomEvent(obj)
{
    qCDebug(E2EE) << "Encrypted event from" << senderId();
}

QString EncryptedEvent::algorithm() const
{
    return contentPart<QString>(AlgorithmKeyL);
}

RoomEventPtr EncryptedEvent::createDecrypted(const QString &decrypted) const
{
    auto eventObject = QJsonDocument::fromJson(decrypted.toUtf8()).object();
    eventObject["event_id"_ls] = id();
    eventObject["sender"_ls] = senderId();
    eventObject["origin_server_ts"_ls] = originTimestamp().toMSecsSinceEpoch();
    if (const auto relatesToJson = contentPart<QJsonObject>("m.relates_to"_ls);
        !relatesToJson.isEmpty()) {
        auto content = eventObject["content"_ls].toObject();
        content["m.relates_to"_ls] = relatesToJson;
        eventObject["content"_ls] = content;
    }
    if (const auto redactsJson = unsignedPart<QString>("redacts"_ls);
        !redactsJson.isEmpty()) {
        auto unsign = eventObject["unsigned"_ls].toObject();
        unsign["redacts"_ls] = redactsJson;
        eventObject["unsigned"_ls] = unsign;
    }
    return loadEvent<RoomEvent>(eventObject);
}

void EncryptedEvent::setRelation(const QJsonObject& relation)
{
    auto content = contentJson();
    content["m.relates_to"_ls] = relation;
    editJson()["content"_ls] = content;
}
