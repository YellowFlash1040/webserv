#pragma once

#ifndef HTTPSTATUSCODE_HPP
# define HTTPSTATUSCODE_HPP

#define HTTP_STATUS_CODES(X) \
	X(None, 0) \
	X(Continue, 100) \
	X(SwitchingProtocols, 101) \
	X(OK, 200) \
	X(Created, 201) \
	X(MovedPermanently, 301) \
	X(Found, 302) \
	X(NotFound, 404) \
	X(InternalServerError, 500) \
	X(NotImplemented, 501)

enum class HttpStatusCode
{
	None,

	// 1xx Informational
	Continue = 100,
	SwitchingProtocols = 101,
	Processing = 102,
	EarlyHints = 103,

	// 2xx Success
	OK = 200,
	Created = 201,
	Accepted = 202,
	NonAuthoritativeInformation = 203,
	NoContent = 204,
	ResetContent = 205,
	PartialContent = 206,
	MultiStatus = 207,
	AlreadyReported = 208,
	ImUsed = 226,

	// 3xx Redirection
	MultipleChoices = 300,
	MovedPermanently = 301,
	Found = 302,
	SeeOther = 303,
	NotModified = 304,
	UseProxy = 305,
	TemporaryRedirect = 307,
	PermanentRedirect = 308,

	// 4xx Client Errors
	BadRequest = 400,
	Unauthorized = 401,
	PaymentRequired = 402,
	Forbidden = 403,
	NotFound = 404,
	MethodNotAllowed = 405,
	NotAcceptable = 406,
	ProxyAuthenticationRequired = 407,
	RequestTimeout = 408,
	Conflict = 409,
	Gone = 410,
	LengthRequired = 411,
	PreconditionFailed = 412,
	PayloadTooLarge = 413,
	UriTooLong = 414,
	UnsupportedMediaType = 415,
	RangeNotSatisfiable = 416,
	ExpectationFailed = 417,
	ImATeapot = 418,
	MisdirectedRequest = 421,
	UnprocessableEntity = 422,
	Locked = 423,
	FailedDependency = 424,
	TooEarly = 425,
	UpgradeRequired = 426,
	PreconditionRequired = 428,
	TooManyRequests = 429,
	RequestHeaderFieldsTooLarge = 431,
	UnavailableForLegalReasons = 451,

	// 5xx Server Errors
	InternalServerError = 500,
	NotImplemented = 501,
	BadGateway = 502,
	ServiceUnavailable = 503,
	GatewayTimeout = 504,
	HttpVersionNotSupported = 505,
	VariantAlsoNegotiates = 506,
	InsufficientStorage = 507,
	LoopDetected = 508,
	NotExtended = 510,
	NetworkAuthenticationRequired = 511

};

// Inline function to map HttpStatusCode -> string using the macro list
inline std::string codeToText(HttpStatusCode code)
{
	switch (code){
#define X(name, value) case HttpStatusCode::name: return #name;
		HTTP_STATUS_CODES(X)
#undef X
		default: return "Unknown";
	}
}

#endif
