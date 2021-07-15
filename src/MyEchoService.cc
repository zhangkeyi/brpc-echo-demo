#include <brpc/server.h>
#include <gflags/gflags.h>

# include "echo.pb.h"

DEFINE_bool(echo_attachment, true, "Echo attachment as well");
DEFINE_int32(port, 8000, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
             "(waiting for client to close connection before server stops)");

class MyEchoService : public EchoService {
    public:
        void Echo (::google::protobuf::RpcController* cntl_base,
                    const EchoRequest* request,
                    EchoResponse* response,
                    ::google::protobuf::Closure* done) 
        {
            brpc::ClosureGuard done_guard (done);

            brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);

            // The purpose of following logs is to help you to understand
            // how clients interact with servers more intuitively. You should 
            // remove these logs in performance-sensitive servers.
            LOG(INFO) << "Received request[log_id=" << cntl->log_id() 
                    << "] from " << cntl->remote_side() 
                  << " to " << cntl->local_side()
                  << ": " << request->message()
                  << " (attached=" << cntl->request_attachment() << ")";

            response -> set_message (request -> message());

            // You can compress the response by setting Controller, but be aware
            // that compression may be costly, evaluate before turning on.
            // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);

            if (FLAGS_echo_attachment) {
                // Set attachment which is wired to network directly instead of
                // being serialized into protobuf messages.
                cntl->response_attachment().append(cntl->request_attachment());
            }


        }
};

int main (int argc, char* argv[]) 
{
    brpc::Server server;

    MyEchoService echo_service_impl;

    // add service
    if (server.AddService(&echo_service_impl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0)
    {
        std::cout << "Failed to add service";
        return -1;
    }
    // start the server
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) !=0)
    {
        std::cout << "Failed to start EchoServer";
        return -1;
    }

    server.RunUntilAskedToQuit();
    return 0;
}