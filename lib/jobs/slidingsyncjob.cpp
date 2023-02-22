// SPDX-FileCopyrightText: 2016 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "slidingsyncjob.h"
#include "converters.h"
#include "event.h"
#include "quotient_common.h"
#include "slidingsyncdata.h"

using namespace Quotient;

static size_t jobId = 0;

SlidingSyncJob::SlidingSyncJob(const QString& txnId, QString pos)
    : BaseJob(HttpVerb::Post, QStringLiteral("SlidingSyncJob-%1").arg(++jobId),
              "_matrix/client/v3/sync")
{
    QUrlQuery query;
    if (pos > -1)
        query.addQueryItem("pos", pos);
    setRequestQuery(query);

    QJsonObject requestData {
        {"txn_id"_ls, txnId},
        {"delta_token"_ls, ""_ls},
        {"lists"_ls, QJsonObject {
            {"0"_ls, QJsonObject {
                {"ranges"_ls, QJsonArray { QJsonArray {0, 20}, QJsonArray {20, 70}}},
                {"sort", QJsonArray {"by_notification_level", "by_recency", "by_name"}},
                {"required_state", QJsonArray {
                    QJsonArray {"m.room.join_rules", ""},
                    QJsonArray {"m.room.avatar", ""},
                    QJsonArray {"m.room.name", ""},
                    }},
                {"timeline_limit", 1},
                //{"filters", QJsonObject {
                //     {"is_dm", false},
                //}}
            }}
        }},
        {"room_subscriptions", QJsonObject{}},
        {"unsubscribe_rooms", QJsonArray{}},
        {"extensions", QJsonObject{}},
    };
    qWarning() << requestData;

    auto request = QStringLiteral("{\"lists\":{\"0\":{\"ranges\":[[0,20]],\"filters\":{\"is_dm\":true},\"required_state\":[[\"m.room.avatar\",\"\"],[\"m.room.tombstone\",\"\"]],\"timeline_limit\":1,\"sort\":[\"by_highlight_count\",\"by_notification_count\",\"by_recency\"]},\"1\":{\"ranges\":[[0,20]],\"filters\":{\"is_dm\":false},\"required_state\":[[\"m.room.avatar\",\"\"],[\"m.room.tombstone\",\"\"]],\"timeline_limit\":1,\"sort\":[\"by_highlight_count\",\"by_notification_count\",\"by_recency\"]}}}");
    requestData = QJsonDocument::fromJson(request.toLatin1()).object();
    setRequestData(requestData);
    setMaxRetries(std::numeric_limits<int>::max());
}

BaseJob::Status SlidingSyncJob::prepareResult()
{
    d.parseJson(jsonData());
    //if (Q_LIKELY(d.unresolvedRooms().isEmpty())) //TODO is this relevant in a sliding world?
    return Success;
}