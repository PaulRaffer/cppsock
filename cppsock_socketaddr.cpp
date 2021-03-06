/**
 *  author: Clemens Pruggmayer (PrugClem)
 *  date:   2020-12-22
 *  desc:   implementation for socketaddr, used to address an(other) computer
 */
#include "cppsock.hpp"

using namespace cppsock;

bool socketaddr::is(sa_family_t family, const std::string &ip)
{
    sockaddr_storage ss;
    return inet_pton(family, ip.data(), &ss) == 1;
}

bool socketaddr::is_ipv4(const std::string &ip)
{
    return is(AF_INET, ip);
}

bool socketaddr::is_ipv6(const std::string &ip)
{
    return is(AF_INET6, ip);
}

socketaddr::socketaddr()
{
    memset(&this->sa, 0, sizeof(this->sa));
}
socketaddr::socketaddr(const socketaddr& other)
{
    memcpy(&this->sa, &other.sa, sizeof(socketaddr::sa));
}
socketaddr::socketaddr(const std::string& addr, uint16_t port)
{
    this->set(addr, port);
}
socketaddr& socketaddr::operator=(const socketaddr& other)
{
    memcpy(&this->sa, &other.sa, sizeof(socketaddr::sa));
    return *this;
}

sockaddr *socketaddr::data()
{
    return &this->sa.sa;
}
const sockaddr *socketaddr::data() const
{
    return &this->sa.sa;
}

void socketaddr::set_family(sa_family_t fam)
{
    memset(&this->sa, 0, sizeof(this->sa));
    this->sa.sa_family = fam;
}

error_t socketaddr::set_addr(const std::string &ip)
{
    void *addrptr;
    if(this->sa.sa_family == AF_INET)
    {
        addrptr = &this->sa.sin.sin_addr;
    }
    else if(this->sa.sa_family == AF_INET6)
    {
        addrptr = &this->sa.sin6.sin6_addr;
    }
    else
    {
        return errno = EAFNOSUPPORT;
    }
    if(inet_pton(this->sa.sa_family, ip.data(), addrptr) == 0) {return errno = EINVAL;}
    return 0;
}

error_t socketaddr::set_port(uint16_t port)
{
    in_port_t *portaddr;
    if(this->sa.sa_family == AF_INET)
    {
        portaddr = &this->sa.sin.sin_port;
    }
    else if(this->sa.sa_family == AF_INET6)
    {
        portaddr = &this->sa.sin6.sin6_port;
    }
    else
    {
        return errno = EAFNOSUPPORT;
    }
    *portaddr = cppsock::hton<uint16_t>(port);
    return 0;
}

error_t socketaddr::set(const std::string& addr, uint16_t port)
{
    error_t error;
    if( is(AF_INET, addr) )
        this->set_family(AF_INET);
    else if( is(AF_INET6, addr) )
        this->set_family(AF_INET6);
    else
        return errno = EAFNOSUPPORT;

    if( (error = this->set_addr(addr)) != 0 )
        return error; // errno is already set from set_addr
    if( (error = this->set_port(port)) != 0 )
        return error; // errno is already set from set_port

    return 0;
}

void socketaddr::set(const sockaddr* data)
{
    this->set_family(AF_UNSPEC);
    if(data->sa_family == AF_INET) // copy IPv4 address structure
        memcpy(&this->sa.sin, data, sizeof(this->sa.sin));
    else if(data->sa_family == AF_INET6) // copy IPv6 address structure
        memcpy(&this->sa.sin6, data, sizeof(this->sa.sin6));
    return;
}

void socketaddr::get_family(sa_family_t &fam) const
{
    fam = this->sa.sa_family;
    return;
}

error_t socketaddr::get_addr(std::string &buf) const
{
    void const *addrptr;
    if(this->sa.sa_family == AF_INET)
    {
        addrptr = &this->sa.sin.sin_addr;
    }
    else if(this->sa.sa_family == AF_INET6)
    {
        addrptr = &this->sa.sin6.sin6_addr;
    }
    else
    {
        return errno = EAFNOSUPPORT;
    }
    buf.resize(INET6_ADDRSTRLEN); // preallocate memory for address
    inet_ntop(this->sa.sa_family, (void*)addrptr, &buf[0], INET6_ADDRSTRLEN); // typecast to void* is needed because windows is stupid and needs a non-const pointer
    buf = buf.c_str(); // reinterpret string to cut additional '\0' characters at the end since this can cause problems
    return 0;
}

error_t socketaddr::get_port(uint16_t &port) const
{
    if(this->sa.sa_family == AF_INET)
    {
        port = cppsock::ntoh<uint16_t>(this->sa.sin.sin_port);
    }
    else if(this->sa.sa_family == AF_INET6)
    {
        port = cppsock::ntoh<uint16_t>(this->sa.sin6.sin6_port);
    }
    else
    {
        return EAFNOSUPPORT;
    }
    return 0;
}

sa_family_t socketaddr::get_family() const
{
    return this->sa.sa_family;
}

std::string socketaddr::get_addr() const
{
    std::string ret;
    this->get_addr(ret);
    return ret;
}

uint16_t socketaddr::get_port() const
{
    uint16_t ret(0);
    this->get_port(ret);
    return ret;
}

bool cppsock::socketaddr::operator==(const socketaddr& other) const
{
    return memcmp(&this->sa, &other.sa, sizeof(this->sa)) == 0;
}

bool cppsock::socketaddr::operator!=(const socketaddr& other) const
{
    return memcmp(&this->sa, &other.sa, sizeof(this->sa)) != 0;
}

bool cppsock::socketaddr::operator<(const socketaddr &other) const
{
    return memcmp(&this->sa, &other.sa, sizeof(this->sa)) < 0;
}

std::ostream& cppsock::operator<<(std::ostream & o, const cppsock::socketaddr& addr)
{
    if(addr.get_family() == AF_INET)
        return o << addr.get_addr() << ":" << addr.get_port();
    else if(addr.get_family() == AF_INET6)
        return o << "[" << addr.get_addr() << "]:" << addr.get_port();
    else
        return o << "error: unknown address family";
}