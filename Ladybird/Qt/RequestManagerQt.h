/*
 * Copyright (c) 2022, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Loader/ResourceLoader.h>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace Ladybird {

class RequestManagerQt
    : public QObject
    , public Web::ResourceLoaderConnector {
    Q_OBJECT
public:
    static NonnullRefPtr<RequestManagerQt> create(Vector<ByteString> const& certificate_paths)
    {
        return adopt_ref(*new RequestManagerQt(certificate_paths));
    }

    virtual ~RequestManagerQt() override { }

    virtual void prefetch_dns(URL::URL const&) override { }
    virtual void preconnect(URL::URL const&) override { }

    virtual RefPtr<Web::ResourceLoaderConnectorRequest> start_request(ByteString const& method, URL::URL const&, HTTP::HeaderMap const& request_headers, ReadonlyBytes request_body, Core::ProxyData const&) override;
    virtual RefPtr<Web::WebSockets::WebSocketClientSocket> websocket_connect(const URL::URL&, ByteString const& origin, Vector<ByteString> const& protocols) override;

private slots:
    void reply_finished(QNetworkReply*);

private:
    explicit RequestManagerQt(Vector<ByteString> const& certificate_paths);

    class Request
        : public Web::ResourceLoaderConnectorRequest {
    public:
        static ErrorOr<NonnullRefPtr<Request>> create(QNetworkAccessManager& qnam, ByteString const& method, URL::URL const& url, HTTP::HeaderMap const& request_headers, ReadonlyBytes request_body, Core::ProxyData const&);

        virtual ~Request() override;

        virtual void set_buffered_request_finished_callback(Requests::Request::BufferedRequestFinished) override;
        virtual void set_unbuffered_request_callbacks(Requests::Request::HeadersReceived, Requests::Request::DataReceived, Requests::Request::RequestFinished) override;
        virtual bool stop() override { return false; }

        void did_finish();

        QNetworkReply& reply() { return m_reply; }

    private:
        Request(QNetworkReply&);

        QNetworkReply& m_reply;

        Requests::Request::BufferedRequestFinished on_buffered_request_finish;
    };

    HashMap<QNetworkReply*, NonnullRefPtr<Request>> m_pending;
    QNetworkAccessManager* m_qnam { nullptr };
};

}
