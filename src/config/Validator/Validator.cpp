#include "Validator.hpp"

// ---------------------------METHODS-----------------------------

void Validator::validate(const std::unique_ptr<Directive>& rootNode)
{
    const BlockDirective* block
        = dynamic_cast<const BlockDirective*>(rootNode.get());
    const std::string& name = rootNode->name();
    if (!block || name != "global")
        throw std::logic_error(
            "The root node has to be a block directive 'global'");

    validateChildren(block);
}

void Validator::validateChildren(const BlockDirective* block)
{
    std::set<std::string> seenDirectives;

    for (const auto& directive : block->directives())
    {
        makeDuplicateCheck(seenDirectives, directive);
        makeConflictsCheck(seenDirectives, directive);
    }

    for (const auto& directive : block->directives())
        validateDirective(directive, block->name());

    if (block->name() == Directives::GLOBAL_CONTEXT)
        expectRequiredDirective(Directives::HTTP, block);

    if (block->name() == Directives::HTTP)
        expectRequiredDirective(Directives::SERVER, block);

    if (block->name() == Directives::SERVER)
    {
        // expectRequiredDirective(Directives::LISTEN, block);
        checkForDuplicateListen(block);
        checkForDuplicateLocationPaths(block);
    }
}

void Validator::expectRequiredDirective(const std::string& requiredDirective,
                                        const BlockDirective* context)
{
    for (const auto& directive : context->directives())
        if (directive->name() == requiredDirective)
            return;

    throw MissingRequiredDirectiveException(requiredDirective, context->name());
}

void Validator::checkForDuplicateLocationPaths(
    const BlockDirective* serverBlock)
{
    std::set<std::string> seenPaths;

    for (const auto& directive : serverBlock->directives())
    {
        if (directive->name() != Directives::LOCATION)
            continue;

        const std::string& path = directive->args()[0];
        bool inserted = seenPaths.insert(path).second;
        if (!inserted)
            throw DuplicateLocationPathException(directive, path);
    }
}

void Validator::checkForDuplicateListen(const BlockDirective* serverBlock)
{
    std::set<std::string> seenListen;

    for (const auto& directive : serverBlock->directives())
    {
        if (directive->name() != Directives::LISTEN)
            continue;

        const std::string& value = directive->args()[0];
        bool inserted = seenListen.insert(value).second;
        if (!inserted)
            throw DuplicateListenException(directive, value);
    }
}

void Validator::makeDuplicateCheck(std::set<std::string>& seenDirectives,
                                   const std::unique_ptr<Directive>& directive)
{
    const std::string& name = directive->name();
    bool inserted = seenDirectives.insert(name).second;
    bool allowsDuplicates = Directives::allowsDuplicates(name);

    if (!inserted && !allowsDuplicates)
        throw DuplicateDirectiveException(directive);
}

void Validator::makeConflictsCheck(std::set<std::string>& seenDirectives,
                                   const std::unique_ptr<Directive>& directive)
{
    const std::vector<std::string>& conflictingDirectives
        = Directives::getConflictingDirectives(directive->name());

    for (auto conflictingDirective : conflictingDirectives)

    {
        if (seenDirectives.find(conflictingDirective) != seenDirectives.end())
            throw ConflictingDirectiveException(directive,
                                                conflictingDirective);
    }
}

void Validator::validateDirective(const std::unique_ptr<Directive>& directive,
                                  const std::string& parentContext)
{
    checkIfAllowedDirective(directive, parentContext);
    validateArguments(directive);

    const BlockDirective* block
        = dynamic_cast<const BlockDirective*>(directive.get());
    if (block)
        validateChildren(block);
}

void Validator::checkIfAllowedDirective(
    const std::unique_ptr<Directive>& directive, const std::string& context)
{
    if (!Directives::isAllowedInContext(directive->name(), context))
        throw DirectiveContextException(directive, context);
}

// void Validator::validateArguments(const std::unique_ptr<Directive>&
// directive)
// {
//     const auto& args = directive->args();
//     const auto& argSpecs = Directives::getArgSpecs(directive->name());

//     if (argSpecs.empty())
//     {
//         if (!args.empty())
//             throw NoArgumentsAllowedException(directive);
//         return;
//     }

//     makeDuplicateCheck(args);

//     size_t argIdx = 0;

//     for (const auto& spec : argSpecs)
//     {
//         size_t matchedCount = 0;

//         // Consume all arguments that match this group (up to maxCount)
//         while (argIdx < args.size() && matchedCount < spec.maxCount
//                && tryValidateArgument(spec.possibleTypes, args[argIdx]))
//         {
//             ++matchedCount;
//             ++argIdx;
//         }

//         // Check if we got enough for this group
//         if (matchedCount < spec.minCount)
//         {
//             // If we still have args but they don't match, they're invalid
//             if (argIdx < args.size())
//                 throw InvalidArgumentException(args[argIdx]);
//             // Otherwise we simply don't have enough args
//             throw NotEnoughArgumentsException(directive);
//         }
//     }

//     // Any leftover arguments?
//     if (argIdx < args.size())
//         throw TooManyArgumentsException(directive);
// }

// bool Validator::tryValidateArgument(
//     const std::vector<ArgumentType>& possibleTypes, const Argument& arg)
// {
//     for (ArgumentType type : possibleTypes)
//     {
//         auto it = validators().find(type);
//         if (it == validators().end())
//             throw std::logic_error("No validator found");

//         try
//         {
//             it->second(arg.value());
//             return true;
//         }
//         catch (const std::exception&)
//         {
//             continue;
//         }
//     }
//     return false;
// }

void Validator::validateArguments(const std::unique_ptr<Directive>& directive)
{
    const auto& args = directive->args();
    const auto& argSpecs = Directives::getArgSpecs(directive->name());

    if (argSpecs.empty() && args.empty())
        return;

    checkBasicPreconditions(directive, argSpecs);

    size_t i = 0;
    for (const auto& spec : argSpecs)
        processArgumentsWithSpec(spec, args, i);

    if (i < args.size())
        throw TooManyArgumentsException(directive);
}

void Validator::checkBasicPreconditions(
    const std::unique_ptr<Directive>& directive,
    const std::vector<Directives::ArgumentSpec>& argSpecs)
{
    checkIfArgumentsAreAllowed(directive, argSpecs);
    checkForDuplicateArguments(directive->args());
    checkIfEnoughArguments(directive, argSpecs);
}

void Validator::processArgumentsWithSpec(const Directives::ArgumentSpec& spec,
                                         const std::vector<Argument>& args,
                                         size_t& i)
{
    size_t count = 0;
    while (i < args.size() && count < spec.maxCount
           && validateArgument(spec.possibleTypes, args[i]))
    {
        ++count;
        ++i;
    }

    if (count < spec.minCount)
        throw InvalidArgumentException(args[i]);
}

// void Validator::consumeArgumentsWhileCan(const Directives::ArgumentSpec&
// spec,
//                                          const std::vector<Argument>& args,
//                                          size_t& i, size_t& count)
// {
//     while (i < args.size() && count < spec.maxCount
//            && validateArgument(spec.possibleTypes, args[i]))
//     {
//         ++count;
//         ++i;
//     }
// }

void Validator::checkIfArgumentsAreAllowed(
    const std::unique_ptr<Directive>& directive,
    const std::vector<Directives::ArgumentSpec>& argSpecs)
{
    if (argSpecs.empty() && !directive->args().empty())
        throw NoArgumentsAllowedException(directive);
}

void Validator::checkIfEnoughArguments(
    const std::unique_ptr<Directive>& directive,
    const std::vector<Directives::ArgumentSpec>& argSpecs)
{
    size_t minCount = 0;
    for (const auto& spec : argSpecs)
        minCount += spec.minCount;

    if (directive->args().size() < minCount)
        throw NotEnoughArgumentsException(directive);
}

// void Validator::validateArguments(const std::unique_ptr<Directive>&
// directive)
// {
//     const auto& args = directive->args();
//     const auto& argSpecs = Directives::getArgSpecs(directive->name());

//     if (argSpecs.empty())
//     {
//         if (args.empty())
//             return;
//         throw NoArgumentsAllowedException(directive);
//     }

//     makeDuplicateCheck(args);

//     size_t i = 0;
//     for (const auto& spec : argSpecs)
//     {
//         size_t count = 0;
//         bool hasThrown = false;
//         while (i < args.size() && (!hasThrown || count > spec.maxCount))
//         {
//             try
//             {
//                 validateArgument(spec.possibleTypes, args[i]);
//                 ++count;
//                 ++i;
//             }
//             catch (const std::exception&)
//             {
//                 hasThrown = true;
//             }
//         }

//         if (count < spec.minCount)
//         {
//             if (i < args.size())
//                 throw InvalidArgumentException(args[i]);
//             throw NotEnoughArgumentsException(directive);
//         }

//         if (count > spec.maxCount)
//             throw TooManyArgumentsException(directive);
//     }

//     if (i < args.size())
//         throw InvalidArgumentException(args[i]);
// }

// ***** Thoughts 3 ******
/*
for each group:
    while (more args and under maxCount):
        try validate(arg) for current group
            success → consume it
        catch (invalid):
            if next group exists:
                try validate(arg) with next group
                    success → move to next group
                    fail → throw InvalidArgument
            else:
                throw InvalidArgument

    if count < minCount → NotEnoughArguments
*/

// ***** Thoughts 2 ******
// If argument is valid, :
// - but we've found enough,
// - and there are no more groups - too many arguments
// - and there is one more group,
// - and it throws - too many arguments

// If the argument has thrown,
// - it's either invalid,
// - or it's next group

// If there is no next group - argument is invalid
// otherwise
// if we haven't found enough arguments for the current - not enought
// arguments otherwise go and check with next group If it's valid - continue
// If it's invalid - invalid argument

// ***** Thoughts 1 ******
/*
Technically it's possible to tell if there is an argument missing, or if
it's invalid, by trying to convert it to the next type. So in other words:
if argument failed to convert into group_1 types, but it succeded to convert
into one of the group_2 types, then it means - argument is missing.
Otherwise - it's invalid.

But for now I decided to treat both situations as "Invalid argument"
*/

void Validator::checkForDuplicateArguments(const std::vector<Argument>& args)
{
    std::set<std::string> seenArguments;

    for (const Argument& arg : args)
    {
        // insert() returns {iterator, bool}
        bool inserted = seenArguments.insert(arg.value()).second;

        if (!inserted)
            throw DuplicateArgumentException(arg);
    }
}

bool Validator::validateArgument(const std::vector<ArgumentType>& possibleTypes,
                                 const std::string& value)
{
    for (ArgumentType type : possibleTypes)
    {
        auto it = validators().find(type);
        if (it == validators().end())
            throw std::logic_error("No validator found");

        try
        {
            const auto& validateValue = it->second;
            validateValue(value); // if this succeeds, we're done
            return true;
        }
        catch (const std::exception& ex)
        {
            // argument failed validation for this type, try next type
            continue;
        }
    }

    return false;
    // throw std::invalid_argument("Argument '" + value
    //                             + "' does not match any allowed type");
}

///----------------///
///----------------///
///----------------///

// clang-format off
const std::map<ArgumentType, std::function<void(const std::string&)>>&
Validator::validators()
{
    static const std::map<ArgumentType, std::function<void(const std::string&)>>
        map = {
            {ArgumentType::Integer, validateInteger},
            {ArgumentType::String, validateString},
            {ArgumentType::URL, validateUrl},
            {ArgumentType::StatusCode, validateStatusCode},
            {ArgumentType::DataSize, validateDataSize},
            {ArgumentType::OnOff, validateOnOff},
            {ArgumentType::FolderPath, validateFolderPath},
            {ArgumentType::FilePath, validateFilePath},
            {ArgumentType::NetworkEndpoint, validateNetworkEndpoint},
            {ArgumentType::Ip, validateIp},
            {ArgumentType::Port, validatePort},
            {ArgumentType::HttpMethod, validateHttpMethod},
            {ArgumentType::URI, validateUri},
            {ArgumentType::Name, validateName},
            {ArgumentType::File, validateFile},
            {ArgumentType::FileExtension, validateFileExtension},
            {ArgumentType::BinaryPath, validateBinaryPath}
        };
    return map;
}
// clang-format on

void Validator::validateStatusCode(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("status code can not be empty");

    if (s.find_first_not_of("1234567890") != std::string::npos)
        throw std::invalid_argument("status code has to be a valid number");

    int code = std::stoi(s);
    if (code < 100 || code > 599)
        throw std::invalid_argument("Invalid status code: " + s);
}

void Validator::validateUrl(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("URL can not be empty");

    if (s.find('.') == std::string::npos)
        throw std::invalid_argument("Invalid URL: " + s);
}

void Validator::validateOnOff(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("expected 'on' or 'off'");

    if (s != "on" && s != "off")
        throw std::invalid_argument("Expected 'on' or 'off', got: " + s);
}

void Validator::validateFilePath(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("file path cannot be empty");

    if (s[0] != '/')
        throw std::invalid_argument("FilePath has to start with '/'");
    // if (s.find('.') == std::string::npos)
    //     throw std::invalid_argument("File has to have an extension");
}

void Validator::validateFolderPath(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("folder path cannot be empty");

    if (s[0] != '/')
        throw std::invalid_argument("Folder path has to start from a '/'");
}

void Validator::validateInteger(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("integer argument can not be empty");

    std::stoi(s);
}

void Validator::validateDataSize(const std::string& s)
{
    Converter::toBodySize(s);
}

void Validator::validateNetworkEndpoint(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument(
            "argument for 'listen' directive can not be empty");
}

void Validator::validateHttpMethod(const std::string& s)
{
    Converter::toHttpMethod(s);
}

void Validator::validateString(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("Argument cannot be empty");
    if (s.front() == '"' && s.back() == '"')
        return;
    if (s.front() == '\'' && s.back() == '\'')
        return;
    if (s.find_first_not_of("0123456789") == std::string::npos)
        throw std::invalid_argument("It looks like it's a number");
}

void Validator::validateUri(const std::string& s)
{
    if (s.empty() || s[0] != '/')
        throw std::invalid_argument("Invalid URI: '" + s + "'");
}

void Validator::validateName(const std::string& s)
{
    (void)s;
    return;
    // if (s.empty())
    //     throw std::invalid_argument("Argument cannot be empty");

    // if (s.find_first_of("/\\") != std::string::npos)
    //     throw std::invalid_argument(
    //         "Name cannot contain '/' or '\\'. Invalid name: '" + s +
    //         "'");

    // for (char c : s)
    // {
    //     if (!(std::isalnum(c) || c == '.'))
    //         throw std::invalid_argument("Name can only contain
    //         alphanumeric "
    //                                     "characters and dots. Invalid
    //                                     name:
    //                                     '"
    //                                     + s + "'");
    // }
}

void Validator::validateFile(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument("Argument cannot be empty");

    if (s.find(".") == std::string::npos)
        throw std::invalid_argument("File has to have an extension");

    if (s.find_first_of("/\\") != std::string::npos)
        throw std::invalid_argument(
            "File cannot contain '/' or '\\'. Invalid file: '" + s + "'");

    for (char c : s)
    {
        if (!(std::isalnum(c) || c == '.' || c == '_' || c == ' '))
            throw std::invalid_argument("File can only contain alphanumeric "
                                        "characters and dots. Invalid file: '"
                                        + s + "'");
    }
}

void Validator::validateIp(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument(
            "argument for 'listen' directive can not be empty");

    (void)s;
    return;
}

void Validator::validatePort(const std::string& s)
{
    if (s.empty())
        throw std::invalid_argument(
            "argument for 'listen' directive can not be empty");

    int number = std::stoi(s);
    if (number < 0 || number > 65535)
        throw std::invalid_argument(
            "Valid port has to be an integer value between 0 and 65535");
}

void Validator::validateFileExtension(const std::string& s)
{
    if (s[0] != '.')
        throw std::invalid_argument("file extension has to start with a '.'");
}

void Validator::validateBinaryPath(const std::string& s)
{
    static std::string allowedBinariesFolder = "/usr/bin/";

    if (s.compare(0, allowedBinariesFolder.size(), allowedBinariesFolder) != 0)
        throw std::invalid_argument("binaries outside " + allowedBinariesFolder
                                    + " are not allowed");
}

//-------------------------THOUGHTS-------------------------------

// Create a map <directive_name, args_validation_function>
// Create a function for each directive
// Create a class for each possible data type
// Use appropriate string -> class conversions and see the results

///----------------------------------------///
///----------------------------------------///
///----------------------------------------///

void Validator::validate(const HttpBlock& httpBlock)
{
    std::unordered_map<std::string, std::unordered_set<std::string>> listen_map;

    for (auto& server : httpBlock.servers)
    {
        for (auto& listen : server.listen)
        {
            for (auto& name : server.serverName)
            {
                auto& names = listen_map[listen];
                if (!names.insert(name).second)
                    throw DuplicateServerIdException(name, listen);
            }
        }
    }
}
