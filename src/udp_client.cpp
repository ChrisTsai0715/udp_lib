#include "udp_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "log_print.h"

using namespace net_lib;

udp_client::udp_client(reactor &reac)
    :	_reactor(reac)
{

}

udp_client::~udp_client()
{

}

bool udp_client::init(std::shared_ptr<net_handler> handler, const sock_addr_def &addr)
{
    _sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (_sockfd < 0)
    {
        log_print(LOG_PRINT_INFO, "init udp sock fd failed...");
        return false; 
    }

    if (!addr.addr.empty() || addr.port != 0)
    {   
        struct sockaddr_in bind_addr;
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_addr.s_addr = inet_addr(addr.addr.c_str());
        bind_addr.sin_port = htons(addr.port);
        if (bind(_sockfd, (struct sockaddr*)&bind_addr, sizeof(struct sockaddr)) < 0)
		{
            log_print(LOG_PRINT_ERROR, "udp sock bind failed...");
			return false; 
		}
    }   
	//设置为异步
    fcntl(_sockfd, F_SETFL, fcntl(_sockfd, F_GETFL, 0) | O_NONBLOCK);

    return base_client::init(handler, addr);
}

bool udp_client::connect(const sock_addr_def &addr)
{
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(addr.addr.c_str());
	dest_addr.sin_port = htons(addr.port);
 
	::connect(_sockfd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in));
    return true;
}

bool udp_client::disconnect()
{
    return true;
}

bool udp_client::send(void *data, size_t size, const sock_addr_def &addr)
{
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(addr.addr.c_str());
    dest_addr.sin_port = htons(addr.port);

    int32_t ret = sendto(_sockfd, data, size, 0, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr));
    if (ret < 0)
    {
        log_print(LOG_PRINT_ERROR, "udp send failed...");
        return false;
    }

	return true;
}

bool udp_client::recv(void)
{
    #define RECV_BUFFER_SIZE 65535
    char *buffer = new char[RECV_BUFFER_SIZE];
    struct sockaddr_in src_addr;
    socklen_t addr_size = sizeof(src_addr);
    int32_t recv_size = ::recvfrom(get_sockfd(), buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &addr_size);

    if (recv_size < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            _reactor.add_recv_task(std::make_shared<net_task>(shared_from_this()));
        }
        else
        {
            log_print(LOG_PRINT_ERROR, "error occurs in %d of %s, code:%d", __LINE__, __FUNCTION__, errno);
        }
    }
    else
    {
        if (_handler != nullptr)
        {
            _handler->handle(net_handler::HANDLE_TYPE_RECV, shared_from_this(), buffer, (size_t)recv_size);
        }
    }

    free(buffer);
    return true;
}

