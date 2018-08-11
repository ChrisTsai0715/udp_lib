#ifndef _UDP_CLIENT_CAIRUI_H_
#define _UDP_CLIENT_CAIRUI_H_

#include "reactor.h"
#include "base_net.h"

namespace net_lib {

	class udp_client : public base_client
	{
	public:
        udp_client(reactor& reac);
		virtual ~udp_client(void);

	public:
        virtual bool init(std::shared_ptr<net_handler> handler, const sock_addr_def &addr = sock_addr_def());
		virtual bool connect(const sock_addr_def &addr);
        virtual bool disconnect(void);
        virtual bool send(void *data, size_t size, const sock_addr_def &addr = sock_addr_def());
        virtual bool recv(void);

    private:
        reactor &_reactor;
	};

}

#endif
