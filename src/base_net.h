#ifndef _BASE_NET_CAIRUI_H_
#define _BASE_NET_CAIRUI_H_

#include <string>
#include <stdint.h>
#include <memory>

namespace net_lib {

    class base_net;

	typedef struct _sock_addr_def
	{
		_sock_addr_def()
			:	addr(""),
                port(0)
		{}

		std::string addr;
		uint16_t port;
	}sock_addr_def;

    class net_handler
    {
    public:
        typedef enum
        {
            HANDLE_TYPE_SEND,
            HANDLE_TYPE_RECV,
        }handle_type;

        virtual ~net_handler(void){}

        virtual bool handle(handle_type type, std::shared_ptr<base_net> net, void *data, size_t size) = 0;
    };

    class handler_decorator : public net_handler
    {
    public:
        handler_decorator(std::shared_ptr<net_handler> handler)
            :	_handler(handler)
        {

        }
        virtual ~handler_decorator() {}

        virtual bool handle(handle_type type, std::shared_ptr<base_net> net, void *data, size_t size)
        {
            if (_handler != nullptr)
                return _handler->handle(type, net, data, size);

            return true;
        }

    private:
        std::shared_ptr<net_handler> _handler;
    };

    class base_net : public std::enable_shared_from_this<base_net>
    {
    public:
        base_net(void) {}
        virtual ~base_net(void) {}

        int get_sockfd(void) const {return _sockfd;}

        virtual bool init(std::shared_ptr<net_handler> handler, const sock_addr_def &addr = sock_addr_def())
        {
            _handler = handler;
            return true;
        }
        virtual bool send(void *data, size_t size, const sock_addr_def &addr = sock_addr_def()) = 0;
        virtual bool recv(void) = 0;

    protected:
        int _sockfd;
        std::shared_ptr<net_handler> _handler;
    };

    class base_client : public base_net
	{
	public:
        base_client(void) {}
        virtual ~base_client(void) {}

		virtual bool connect(const sock_addr_def &addr) = 0;
        virtual bool disconnect(void) = 0;
	};
}

#endif
