// --------------------------------------------------------------------------
//
// File
//		Name:    HTTPResponse.h
//		Purpose: Response object for HTTP connections
//		Created: 26/3/04
//
// --------------------------------------------------------------------------

#ifndef HTTPRESPONSE__H
#define HTTPRESPONSE__H

#include <string>
#include <vector>

#include "CollectInBufferStream.h"

// --------------------------------------------------------------------------
//
// Class
//		Name:    HTTPResponse
//		Purpose: Response object for HTTP connections
//		Created: 26/3/04
//
// --------------------------------------------------------------------------
class HTTPResponse : public CollectInBufferStream
{
public:
	HTTPResponse();
	~HTTPResponse();
private:
	// no copying
	HTTPResponse(const HTTPResponse &);
	HTTPResponse &operator=(const HTTPResponse &);
public:

	void SetResponseCode(int Code);
	void SetContentType(const char *ContentType);

	void SetAsRedirect(const char *RedirectTo, bool IsLocalURI = true);
	void SetAsNotFound(const char *URI);

	void Send(IOStream &rStream, bool OmitContent = false);

	void AddHeader(const char *EntireHeaderLine);
	void AddHeader(const std::string &rEntireHeaderLine);
	void AddHeader(const char *Header, const char *Value);
	void AddHeader(const char *Header, const std::string &rValue);
	void AddHeader(const std::string &rHeader, const std::string &rValue);

	// Set dynamic content flag, default is content is dynamic
	void SetResponseIsDynamicContent(bool IsDynamic) {mResponseIsDynamicContent = IsDynamic;}
	// Set keep alive control, default is to mark as to be closed
	void SetKeepAlive(bool KeepAlive) {mKeepAlive = KeepAlive;}

	void SetCookie(const char *Name, const char *Value, const char *Path = "/", int ExpiresAt = 0);

	enum
	{
		Code_OK = 200,
		Code_NoContent = 204,
		Code_MovedPermanently = 301,
		Code_Found = 302,	// redirection
		Code_NotModified = 304,
		Code_TemporaryRedirect = 307,
		Code_Unauthorized = 401,
		Code_Forbidden = 403,
		Code_NotFound = 404,
		Code_InternalServerError = 500,
		Code_NotImplemented = 501
	};

	static const char *ResponseCodeToString(int ResponseCode);
	
	void WriteStringDefang(const char *String, unsigned int StringLen);
	void WriteStringDefang(const std::string &rString) {WriteStringDefang(rString.c_str(), rString.size());}

	// --------------------------------------------------------------------------
	//
	// Function
	//		Name:    HTTPResponse::WriteString(const std::string &)
	//		Purpose: Write a string to the response (simple sugar function)
	//		Created: 9/4/04
	//
	// --------------------------------------------------------------------------
	void WriteString(const std::string &rString)
	{
		Write(rString.c_str(), rString.size());
	}

	// --------------------------------------------------------------------------
	//
	// Function
	//		Name:    HTTPResponse::SetDefaultURIPrefix(const std::string &)
	//		Purpose: Set default prefix used to local redirections
	//		Created: 26/3/04
	//
	// --------------------------------------------------------------------------
	static void SetDefaultURIPrefix(const std::string &rPrefix)
	{
		msDefaultURIPrefix = rPrefix;
	}

private:
	int mResponseCode;
	bool mResponseIsDynamicContent;
	bool mKeepAlive;
	std::string mContentType;
	std::vector<std::string> mExtraHeaders;
	
	static std::string msDefaultURIPrefix;
};

#endif // HTTPRESPONSE__H
