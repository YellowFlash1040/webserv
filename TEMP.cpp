// V1
void Validator::validateArguments(const std::unique_ptr<Directive>& directive)
{
    const std::vector<Argument>& args = directive->args();
    const std::vector<Directives::ArgumentSpec>& argSpecs
        = Directives::getArgSpecs(directive->name());

    if (argSpecs.size() == 0 && args.size() > 1)
        throw NoArgumentsAllowedException(directive);

    makeDuplicateCheck(args);

    size_t i = 0;
    for (const Directives::ArgumentSpec& spec : argSpecs)
    {
        size_t count = 0;
        bool hasThrown = false;
        while (i < args.size() && !hasThrown)
        {
            try
            {
                validateArgument(spec.possibleTypes, args[i]);
                ++count;
                if (count > spec.maxCount)
                    throw ExtraArgumentException(args[i]);
                ++i;
            }
            catch (const std::exception&)
            {
                hasThrown = true;
                if (spec.minCount > 0
                    && count < spec.minCount) // if argument is not optional,
                                              // and we haven't found enough yet
                    throw InvalidArgumentException(args[i]);
                if (count > spec.maxCount)
                    throw ExtraArgumentException(args[i]);
            }
        }
        if (spec.minCount > 0
            && count < spec.minCount) // if argument is not optional,
            // and we haven't found enough yet
            throw NotEnoughArgumentsException(directive);
    }
    if (i < args.size())
        throw InvalidArgumentException(args[i]);
}

// V2
void Validator::validateArguments(const std::unique_ptr<Directive>& directive)
{
    const auto& args = directive->args();
    const auto& argSpecs = Directives::getArgSpecs(directive->name());

    if (argSpecs.empty())
    {
        if (!args.empty())
            throw NoArgumentsAllowedException(directive);
        return;
    }

    makeDuplicateCheck(args);

    size_t i = 0;
    for (size_t j = 0; j < argSpecs.size(); ++j)
    {
        const Directives::ArgumentSpec& spec = argSpecs[j];
        size_t count = 0;
        bool hasThrown = false;
        while (i < args.size() && !(hasThrown || count > spec.maxCount))
        {
            try
            {
                validateArgument(spec.possibleTypes, args[i]);
                ++count;
                ++i;
            }
            catch (const std::exception&)
            {
                hasThrown = true;
                // if it throws, there are 2 options:
                // - we found an invalid argument or
                // - we reached the end of the current group
            }
        }

        // if we haven't found enough valid arguments
        if (count < spec.minCount)
        {
            // but we haven't reached the end
            // or there no more specs
            if (i < args.size() || j == argSpecs.size() - 1)
                // we found an invalid argument
                throw InvalidArgumentException(args[i]);
            // otherwise there are just not enough arguments
            throw NotEnoughArgumentsException(directive);
        }
        // if found too much
        if (count > spec.maxCount)
            throw TooManyArgumentsException(directive);

        // if we found the right amount it means that we
        // reached the end of the current group
        // and we need to go to the next one
    }

    // If we are here - it means that all of the arguments so far were valid
    // So if we haven't reached the end
    if (i < args.size())
        // It means that we have too many arguments
        throw TooManyArgumentsException(directive);
}

// V3

void Validator::validateArguments(const std::unique_ptr<Directive>& directive)
{
    const auto& args = directive->args();
    const auto& argSpecs = Directives::getArgSpecs(directive->name());

    if (argSpecs.empty())
    {
        if (!args.empty())
            throw NoArgumentsAllowedException(directive);
        return;
    }

    makeDuplicateCheck(args);

    size_t i = 0;
    for (size_t j = 0; j < argSpecs.size(); ++j)
    {
        const auto& group = argSpecs[j];
        size_t count = 0;
        while (i < args.size() && count < group.maxCount)
        {
            try
            {
                validateArgument(group.possibleTypes, args[i]);
                ++count;
                ++i;
            }
            catch (...)
            {
                if (j + 1 < argSpecs.size())
                {
                    try
                    {
                        const auto& nextGroup = argSpecs[j + 1];
                        validateArgument(nextGroup.possibleTypes, args[i]);
                        break;
                    }
                    catch (...)
                    {
                        throw InvalidArgumentException(args[i]);
                    }
                }
                throw InvalidArgumentException(args[i]);
            }

            if (count < group.minCount)
            {
                if (count == 0)
                    throw MissingArgumentException(directive);
                throw NotEnoughArgumentsException(directive);
            }
        }

        if (count < group.minCount)
        {
            if (count == 0)
                throw MissingArgumentException(directive);
            throw NotEnoughArgumentsException(directive);
        }

        if (i < args.size())
            throw TooManyArgumentsException(directive);
    }
}

// V4

void Validator::validateArguments(const std::unique_ptr<Directive>& directive)
{
    const auto& args = directive->args();
    const auto& argSpecs = Directives::getArgSpecs(directive->name());

    if (argSpecs.empty())
    {
        if (!args.empty())
            throw NoArgumentsAllowedException(directive);
        return;
    }

    makeDuplicateCheck(args);

    size_t i = 0;
    for (size_t j = 0; j < argSpecs.size(); ++j)
    {
        const auto& group = argSpecs[j];
        size_t count = 0;
        while (i < args.size() && count < group.maxCount)
        {
            try
            {
                validateArgument(group.possibleTypes, args[i]);
                ++count;
                ++i;
            }
            catch (...)
            {
                if (j + 1 < argSpecs.size())
                {
                    try
                    {
                        const auto& nextGroup = argSpecs[j + 1];
                        validateArgument(nextGroup.possibleTypes, args[i]);
                        break;
                    }
                    catch (...)
                    {
                        throw InvalidArgumentException(args[i]);
                    }
                }
                throw InvalidArgumentException(args[i]);
            }
        }

        if (count < group.minCount)
            throw NotEnoughArgumentsException(directive);

        if (i < args.size())
            throw TooManyArgumentsException(directive);
    }
}

// V5

void Validator::validateArguments(const std::unique_ptr<Directive>& directive)
{
    const auto& args = directive->args();
    const auto& argSpecs = Directives::getArgSpecs(directive->name());

    if (argSpecs.empty())
    {
        if (!args.empty())
            throw NoArgumentsAllowedException(directive);
        return;
    }

    makeDuplicateCheck(args);

    size_t i = 0;
    for (size_t specIdx = 0; specIdx < argSpecs.size(); ++specIdx)
    {
        const auto& spec = argSpecs[specIdx];
        size_t count = 0;

        // While we have more args and haven't reached maxCount
        while (i < args.size() && count < spec.maxCount)
        {
            try
            {
                // Try to validate current arg with current group
                validateArgument(spec.possibleTypes, args[i]);
                // Success → consume it
                ++count;
                ++i;
            }
            catch (const std::exception&)
            {
                // Invalid for current group
                if (specIdx + 1 < argSpecs.size())
                {
                    // Next group exists, try it
                    try
                    {
                        const auto& nextSpec = argSpecs[specIdx + 1];
                        validateArgument(nextSpec.possibleTypes, args[i]);
                        // Success → move to next group (break out of while
                        // loop)
                        break;
                    }
                    catch (const std::exception&)
                    {
                        // Fail → throw InvalidArgument
                        throw InvalidArgumentException(args[i]);
                    }
                }
                else
                {
                    // No next group → throw InvalidArgument
                    throw InvalidArgumentException(args[i]);
                }
            }
        }

        // Check if we collected enough arguments for this group
        if (count < spec.minCount)
            throw NotEnoughArgumentsException(directive);
    }

    // If some args left → TooManyArguments
    if (i < args.size())
        throw TooManyArgumentsException(directive);
}

/*
for each group:
    while (more args and not enough):
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

if (some args left)
    -> TooManyArguments
*/