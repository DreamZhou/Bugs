bool MLClientScoket::ccConfigSocket()
{
    NetLog::Log("MLClientScoket::ccConfigSocket");
    
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 )
	unsigned long ul = 1;
	int nRet = ioctlsocket(m_uSocket, FIONBIO, &ul);
	if( nRet == SOCKET_ERROR )
	{
		CCLOGERROR("set unblocking failed");
		ccClose();
		return false;
	}
#endif

#if( CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
	int nFlags = fcntl(m_uSocket, F_GETFL, 0);
    int nRet = fcntl(m_uSocket, F_SETFL, nFlags | O_NONBLOCK);
	if( nRet == SOCKET_ERROR )
	{
		CCLOGERROR("set unblocking failed");
		ccClose();
		return false;
	}
#endif

	const char nNoDelay = 1;
	if( setsockopt(m_uSocket, IPPROTO_TCP, TCP_NODELAY, &nNoDelay, sizeof(nNoDelay)) == SOCKET_ERROR )
	{
		CCLOGERROR("set nodelay failed");
		ccClose();
		return false;
	}

	return true;
}

bool MLClientScoket::ccConnect()
{
	NetLog::Log("MLClientScoket::ccConnect");
    ccClose();

    struct addrinfo hints, *res, *res0;
    int error;
    int nRet = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags =  AI_NUMERICSERV ;
    hints.ai_protocol = IPPROTO_TCP;

    error = getaddrinfo(Ip_str(), Port_str(), &hints, &res0);

    if (error) {
        NetLog::Log("getaddrinfo  error");
        return false;
    }

    for (res = res0; res; res = res->ai_next) {
        m_uSocket = socket(res->ai_family, res->ai_socktype,res->ai_protocol);

//      if (m_uSocket == INVALID_SOCKET || !ccConfigSocket()) {  //这样不可以
        if (m_uSocket == INVALID_SOCKET) {                       // 这个可以
            NetLog::Log("m_uSocket == INVALID_SOCKET ");
            continue;
        }

         nRet = ::connect(m_uSocket, res->ai_addr, res->ai_addrlen);
        if (nRet ==-1) {
            ccClose();
            m_uSocket = INVALID_SOCKET;
            continue;
        }
        break;  /* okay we got one */
    }
    freeaddrinfo(res0);
	if( nRet == 0 )
	{
		return true;
	}
	else
	{
#if( CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 )
		int nError = WSAGetLastError();
		if( nError ==  WSAEWOULDBLOCK )
		{
			return true;
		}
		else
		{
			return false;
		}
#endif

#if( CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID )
		if( nRet == SOCKET_ERROR && errno == EINPROGRESS )
		{
			return true;
		}
		else
		{
			return false;
        }
#endif
	}
}