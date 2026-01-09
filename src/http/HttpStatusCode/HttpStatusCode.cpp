#include "HttpStatusCode.hpp"

std::string codeToText(HttpStatusCode code)
{
	switch (code)
	{
		case HttpStatusCode::None: return "None";
		case HttpStatusCode::Continue: return "Continue";
		case HttpStatusCode::SwitchingProtocols: return "Switching Protocols";
		case HttpStatusCode::Processing: return "Processing";
		case HttpStatusCode::EarlyHints: return "Early Hints";
		case HttpStatusCode::OK: return "OK";
		case HttpStatusCode::Created: return "Created";
		case HttpStatusCode::Accepted: return "Accepted";
		case HttpStatusCode::NonAuthoritativeInformation: return "Non-Authoritative Information";
		case HttpStatusCode::NoContent: return "No Content";
		case HttpStatusCode::ResetContent: return "Reset Content";
		case HttpStatusCode::PartialContent: return "Partial Content";
		case HttpStatusCode::MultiStatus: return "Multi Status";
		case HttpStatusCode::AlreadyReported: return "Already Reported";
		case HttpStatusCode::ImUsed: return "IM Used";
		case HttpStatusCode::MultipleChoices: return "Multiple Choices";
		case HttpStatusCode::MovedPermanently: return "Moved Permanently";
		case HttpStatusCode::Found: return "Found";
		case HttpStatusCode::SeeOther: return "See Other";
		case HttpStatusCode::NotModified: return "Not Modified";
		case HttpStatusCode::UseProxy: return "Use Proxy";
		case HttpStatusCode::TemporaryRedirect: return "Temporary Redirect";
		case HttpStatusCode::PermanentRedirect: return "Permanent Redirect";
		case HttpStatusCode::BadRequest: return "Bad Request";
		case HttpStatusCode::Unauthorized: return "Unauthorized";
		case HttpStatusCode::PaymentRequired: return "Payment Required";
		case HttpStatusCode::Forbidden: return "Forbidden";
		case HttpStatusCode::NotFound: return "Not Found";
		case HttpStatusCode::MethodNotAllowed: return "Method Not Allowed";
		case HttpStatusCode::NotAcceptable: return "Not Acceptable";
		case HttpStatusCode::ProxyAuthenticationRequired: return "Proxy Authentication Required";
		case HttpStatusCode::RequestTimeout: return "Request Timeout";
		case HttpStatusCode::Conflict: return "Conflict";
		case HttpStatusCode::Gone: return "Gone";
		case HttpStatusCode::LengthRequired: return "Length Required";
		case HttpStatusCode::PreconditionFailed: return "Precondition Failed";
		case HttpStatusCode::PayloadTooLarge: return "Payload Too Large";
		case HttpStatusCode::UriTooLong: return "URI Too Long";
		case HttpStatusCode::UnsupportedMediaType: return "Unsupported Media Type";
		case HttpStatusCode::RangeNotSatisfiable: return "Range Not Satisfiable";
		case HttpStatusCode::ExpectationFailed: return "Expectation Failed";
		case HttpStatusCode::ImATeapot: return "I'm A Teapot";
		case HttpStatusCode::MisdirectedRequest: return "Misdirected Request";
		case HttpStatusCode::UnprocessableEntity: return "Unprocessable Entity";
		case HttpStatusCode::Locked: return "Locked";
		case HttpStatusCode::FailedDependency: return "Failed Dependency";
		case HttpStatusCode::TooEarly: return "Too Early";
		case HttpStatusCode::UpgradeRequired: return "Upgrade Required";
		case HttpStatusCode::PreconditionRequired: return "Precondition Required";
		case HttpStatusCode::TooManyRequests: return "Too Many Requests";
		case HttpStatusCode::RequestHeaderFieldsTooLarge: return "Request Header Fields Too Large";
		case HttpStatusCode::UnavailableForLegalReasons: return "Unavailable For Legal Reasons";
		case HttpStatusCode::InternalServerError: return "Internal Server Error";
		case HttpStatusCode::NotImplemented: return "Not Implemented";
		case HttpStatusCode::BadGateway: return "Bad Gateway";
		case HttpStatusCode::ServiceUnavailable: return "Service Unavailable";
		case HttpStatusCode::GatewayTimeout: return "Gateway Timeout";
		case HttpStatusCode::HttpVersionNotSupported: return "HTTP Version Not Supported";
		case HttpStatusCode::VariantAlsoNegotiates: return "Variant Also Negotiates";
		case HttpStatusCode::InsufficientStorage: return "Insufficient Storage";
		case HttpStatusCode::LoopDetected: return "Loop Detected";
		case HttpStatusCode::NotExtended: return "Not Extended";
		case HttpStatusCode::NetworkAuthenticationRequired: return "Network Authentication Required";
		default: return "Unknown";
	}
}