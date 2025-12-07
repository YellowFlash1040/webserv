#pragma once

#ifndef HTTPSTATUSCODE_HPP
# define HTTPSTATUSCODE_HPP

#define HTTP_STATUS_CODES(X) \
	/* 0 */ \
	X(None, 0) \
	/* 1xx Informational */ \
	X(Continue, 100) \
	X(SwitchingProtocols, 101) \
	X(Processing, 102) \
	X(EarlyHints, 103) \
	/* 2xx Success */ \
	X(OK, 200) \
	X(Created, 201) \
	X(Accepted, 202) \
	X(NonAuthoritativeInformation, 203) \
	X(NoContent, 204) \
	X(ResetContent, 205) \
	X(PartialContent, 206) \
	X(MultiStatus, 207) \
	X(AlreadyReported, 208) \
	X(ImUsed, 226) \
	/* 3xx Redirection */ \
	X(MultipleChoices, 300) \
	X(MovedPermanently, 301) \
	X(Found, 302) \
	X(SeeOther, 303) \
	X(NotModified, 304) \
	X(UseProxy, 305) \
	X(TemporaryRedirect, 307) \
	X(PermanentRedirect, 308) \
	/* 4xx Client Errors */ \
	X(BadRequest, 400) \
	X(Unauthorized, 401) \
	X(PaymentRequired, 402) \
	X(Forbidden, 403) \
	X(NotFound, 404) \
	X(MethodNotAllowed, 405) \
	X(NotAcceptable, 406) \
	X(ProxyAuthenticationRequired, 407) \
	X(RequestTimeout, 408) \
	X(Conflict, 409) \
	X(Gone, 410) \
	X(LengthRequired, 411) \
	X(PreconditionFailed, 412) \
	X(PayloadTooLarge, 413) \
	X(UriTooLong, 414) \
	X(UnsupportedMediaType, 415) \
	X(RangeNotSatisfiable, 416) \
	X(ExpectationFailed, 417) \
	X(ImATeapot, 418) \
	X(MisdirectedRequest, 421) \
	X(UnprocessableEntity, 422) \
	X(Locked, 423) \
	X(FailedDependency, 424) \
	X(TooEarly, 425) \
	X(UpgradeRequired, 426) \
	X(PreconditionRequired, 428) \
	X(TooManyRequests, 429) \
	X(RequestHeaderFieldsTooLarge, 431) \
	X(UnavailableForLegalReasons, 451) \
	/* 5xx Server Errors */ \
	X(InternalServerError, 500) \
	X(NotImplemented, 501) \
	X(BadGateway, 502) \
	X(ServiceUnavailable, 503) \
	X(GatewayTimeout, 504) \
	X(HttpVersionNotSupported, 505) \
	X(VariantAlsoNegotiates, 506) \
	X(InsufficientStorage, 507) \
	X(LoopDetected, 508) \
	X(NotExtended, 510) \
	X(NetworkAuthenticationRequired, 511)

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
